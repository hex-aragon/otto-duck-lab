//! otto_moves — 오또 움직임 기록을 체인 위에서 검사하고 보상(MOVE)을 주는 프로그램
//!
//! 핵심: `otto`는 NU40 보드 안의 ed25519 키다. 모든 기록 트랜잭션에 `otto`가 Signer로 들어가므로,
//! 솔라나 런타임이 NU40 서명을 직접 검사한다. 보드 없이 만든 기록은 거부된다.
//!
//! 명령
//!   init          관리자가 한 번: 설정 + MOVE 토큰 민트(PDA가 발행 권한)
//!   register      오또 등록: NU40이 서명 → 오또 프로필 PDA (주인 = 등록한 지갑)
//!   record_moment 움직임 기록: NU40 서명 + 원본 SHA-256 + 요약 → 기록 PDA + MOVE 보상
//!
//! PDA
//!   ["config"]                        설정
//!   ["move"]                          MOVE 민트 (소수점 0)
//!   ["otto", otto]                    오또 프로필
//!   ["moment", otto, 번호(u32 LE)]    기록 하나

use anchor_lang::prelude::*;
use anchor_spl::associated_token::AssociatedToken;
use anchor_spl::token_interface::{self, Mint, MintTo, TokenAccount, TokenInterface};

pub mod score;
use score::Motion;

declare_id!("11111111111111111111111111111111"); // 배포할 때 `anchor keys sync`로 바뀜

/// 같은 오또가 연달아 기록할 수 있는 최소 간격(초)
pub const MIN_INTERVAL: i64 = 30;

#[program]
pub mod otto_moves {
    use super::*;

    pub fn init(ctx: Context<Init>) -> Result<()> {
        let c = &mut ctx.accounts.config;
        c.admin = ctx.accounts.admin.key();
        c.move_mint = ctx.accounts.move_mint.key();
        c.ottos = 0;
        c.moments = 0;
        c.bump = ctx.bumps.config;
        Ok(())
    }

    pub fn register(ctx: Context<Register>, name: String) -> Result<()> {
        require!(name.len() <= 32, OttoError::NameTooLong);
        let p = &mut ctx.accounts.profile;
        p.otto = ctx.accounts.otto.key();
        p.owner = ctx.accounts.owner.key();
        p.name = name;
        p.moments = 0;
        p.total_reward = 0;
        p.last_ts = 0;
        p.bump = ctx.bumps.profile;
        ctx.accounts.config.ottos += 1;
        emit!(OttoRegistered { otto: p.otto, owner: p.owner });
        Ok(())
    }

    pub fn record_moment(ctx: Context<RecordMoment>, sha256: [u8; 32], motion: MotionArgs) -> Result<()> {
        let now = Clock::get()?.unix_timestamp;
        let p = &mut ctx.accounts.profile;
        require!(p.last_ts == 0 || now - p.last_ts >= MIN_INTERVAL, OttoError::TooSoon);
        require!(sha256 != [0u8; 32], OttoError::EmptyHash);

        let m: Motion = motion.into();
        let reward = score::reward(&m);

        let mo = &mut ctx.accounts.moment;
        mo.otto = p.otto;
        mo.index = p.moments;
        mo.sha256 = sha256;
        mo.motion = motion;
        mo.reward = reward;
        mo.ts = now;
        mo.slot = Clock::get()?.slot;

        if reward > 0 {
            let seeds: &[&[u8]] = &[b"config", &[ctx.accounts.config.bump]];
            token_interface::mint_to(
                CpiContext::new_with_signer(
                    ctx.accounts.token_program.to_account_info(),
                    MintTo {
                        mint: ctx.accounts.move_mint.to_account_info(),
                        to: ctx.accounts.otto_move.to_account_info(),
                        authority: ctx.accounts.config.to_account_info(),
                    },
                    &[seeds],
                ),
                reward,
            )?;
        }

        p.moments += 1;
        p.total_reward += reward;
        p.last_ts = now;
        ctx.accounts.config.moments += 1;
        emit!(MomentRecorded { otto: p.otto, index: mo.index, sha256, reward });
        Ok(())
    }
}

// ───────── 계정 ─────────

#[account]
#[derive(InitSpace)]
pub struct Config {
    pub admin: Pubkey,
    pub move_mint: Pubkey,
    pub ottos: u64,
    pub moments: u64,
    pub bump: u8,
}

#[account]
#[derive(InitSpace)]
pub struct OttoProfile {
    pub otto: Pubkey,  // NU40 지갑 (기기 신분증)
    pub owner: Pubkey, // 등록한 사람 지갑
    #[max_len(32)]
    pub name: String,
    pub moments: u32,
    pub total_reward: u64,
    pub last_ts: i64,
    pub bump: u8,
}

#[account]
#[derive(InitSpace)]
pub struct Moment {
    pub otto: Pubkey,
    pub index: u32,
    pub sha256: [u8; 32], // 원본 기록 해시 → 웹 "🔒 대조"와 같은 값
    pub motion: MotionArgs,
    pub reward: u64,
    pub ts: i64,
    pub slot: u64,
}

#[derive(AnchorSerialize, AnchorDeserialize, Clone, Copy, Debug, Default, InitSpace)]
pub struct MotionArgs {
    pub sec: u32,
    pub steps: u32,
    pub dance_sec: u32,
    pub max_tilt: u16,
    pub falls: u16,
    pub lines: u32,
}

impl From<MotionArgs> for Motion {
    fn from(a: MotionArgs) -> Self {
        Motion { sec: a.sec, steps: a.steps, dance_sec: a.dance_sec, max_tilt: a.max_tilt, falls: a.falls, lines: a.lines }
    }
}

// ───────── 명령별 계정 목록 ─────────

#[derive(Accounts)]
pub struct Init<'info> {
    #[account(mut)]
    pub admin: Signer<'info>,
    #[account(init, payer = admin, space = 8 + Config::INIT_SPACE, seeds = [b"config"], bump)]
    pub config: Account<'info, Config>,
    #[account(init, payer = admin, seeds = [b"move"], bump, mint::decimals = 0, mint::authority = config, mint::token_program = token_program)]
    pub move_mint: InterfaceAccount<'info, Mint>,
    pub token_program: Interface<'info, TokenInterface>,
    pub system_program: Program<'info, System>,
}

#[derive(Accounts)]
pub struct Register<'info> {
    /// NU40 보드 키: 이 서명이 없으면 등록 안 됨
    pub otto: Signer<'info>,
    #[account(mut)]
    pub owner: Signer<'info>,
    #[account(mut, seeds = [b"config"], bump = config.bump)]
    pub config: Account<'info, Config>,
    #[account(init, payer = owner, space = 8 + OttoProfile::INIT_SPACE, seeds = [b"otto", otto.key().as_ref()], bump)]
    pub profile: Account<'info, OttoProfile>,
    pub system_program: Program<'info, System>,
}

#[derive(Accounts)]
pub struct RecordMoment<'info> {
    /// NU40 보드 키: 런타임이 서명을 검사한다
    pub otto: Signer<'info>,
    /// 수수료 · 계정 임대료를 내는 지갑 (웹의 수수료 지갑)
    #[account(mut)]
    pub payer: Signer<'info>,
    #[account(mut, seeds = [b"config"], bump = config.bump, has_one = move_mint)]
    pub config: Account<'info, Config>,
    #[account(mut, seeds = [b"otto", otto.key().as_ref()], bump = profile.bump, has_one = otto)]
    pub profile: Account<'info, OttoProfile>,
    #[account(init, payer = payer, space = 8 + Moment::INIT_SPACE,
        seeds = [b"moment", otto.key().as_ref(), &profile.moments.to_le_bytes()], bump)]
    pub moment: Account<'info, Moment>,
    #[account(mut)]
    pub move_mint: InterfaceAccount<'info, Mint>,
    /// 보상은 오또 지갑(NU40)의 MOVE 계정으로
    #[account(init_if_needed, payer = payer, associated_token::mint = move_mint, associated_token::authority = otto,
        associated_token::token_program = token_program)]
    pub otto_move: InterfaceAccount<'info, TokenAccount>,
    pub token_program: Interface<'info, TokenInterface>,
    pub associated_token_program: Program<'info, AssociatedToken>,
    pub system_program: Program<'info, System>,
}

// ───────── 이벤트 · 오류 ─────────

#[event]
pub struct OttoRegistered {
    pub otto: Pubkey,
    pub owner: Pubkey,
}

#[event]
pub struct MomentRecorded {
    pub otto: Pubkey,
    pub index: u32,
    pub sha256: [u8; 32],
    pub reward: u64,
}

#[error_code]
pub enum OttoError {
    #[msg("이름은 32바이트까지")]
    NameTooLong,
    #[msg("같은 오또는 30초에 한 번만 기록할 수 있어요")]
    TooSoon,
    #[msg("원본 해시가 비어 있어요")]
    EmptyHash,
}
