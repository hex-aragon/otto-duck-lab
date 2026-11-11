> **변경 고지 (Apache-2.0 §4):** 이 파일은 [apirrone/Open_Duck_Mini](https://github.com/apirrone/Open_Duck_Mini) (Apache License 2.0) 원문을 한국어로 번역하고 "번역자 참고"를 덧붙인 수정본이다. 라이선스 전문: [LICENSE-Apache-2.0.txt](../../LICENSE-Apache-2.0.txt). 원저작자와 관계없는 비공식 번역이다.
>
> **번역본입니다.** 원문: [Open_Duck_Mini/docs/feetech_identification.md](https://github.com/apirrone/Open_Duck_Mini/blob/v2/docs/feetech_identification.md) · 번역일 2026-11-02 · 명령과 코드는 원문 그대로이며 주석만 옮겼습니다.
>
> **번역자 참고**
> - 원문은 수식 메모입니다. 각 기호의 뜻을 자세히 설명하지 않아 원문 범위 안에서만 옮겼습니다.

$\theta_d^f$ : 펌웨어 단위로 요청한 $\theta$

$\epsilon = \theta_d^f - \theta^f$

### 펌웨어 단위의 오차

> $\lambda$는 듀티 사이클, 즉 **PWM**입니다.



$\epsilon^f = \epsilon \frac{4096}{2\pi}$

> 12비트

### 펌웨어 듀티 사이클


$\lambda^f = K_p \epsilon^f$

$\lambda = K_p K_g\epsilon$


$R = 2.5\Omega$

------

$\lambda = \epsilon K_p K_g$

$K_g = \frac{\lambda}{\epsilon K_p }$
