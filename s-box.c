#include<stdio.h>
/* <다항식으로 비트를 표현하는 것에 대한 정의>
이제 우리가 s-box를 연산을 할껀데 이는 GF(2^8) 에서의 연산이기 떄문에 다항식 연산을 해야한다. 
그래서 우리가 비트를 이용하여 다항식 연산을 진행하는데 우리는 다항식을 어떻게 표현할 것인지 정의를 할것이다. 
<Definition>
우리는 내림차순으로 정의를 할것이다. 
수식으로 표현하면 다음과 같이 표현할 수 있다.
sum_{i=0}^{n} a_ix^i => a_n a_{n-1} ... a_1 a_0
예를 들어서 0x11b = 0001 0001 1011 을 다항식으로 표현하면 x^8 + x^4 + x^3 + x + 1 으로 표현 할 수 있다. 

<Definition>
우리는 이제 GF(2^8) 에서의 연산을 진행을 할껀데 GF(2^8)을 생성할때 사용되는 8차 기약다항식을 우리는 
f(x) = x^8 + x^4 + x^3 + x + 1 로 정하고 시작을 할것이다. 

*/

//최대 차수를 구하는 함수
int Count_degree(unsigned int a) {
	/* <주어진 다항식의 최고 차수를 계산하는 함수>	
	 32비트 에서 최고차항의 위치를 파악하기 위해 카운트를 해준다
		ex) 최고차항이 x^10 이다 하면 0000 0000 0000 0000 0000 0100 0000 0000 인데
		(1000 0000 0000 0000 0000 0000 0000 0000) & (0000 0000 0000 0000 0000 0100 0000 0000)  == 1000 0000 0000 0000 0000 0000 0000 0000 이 될때까지 
		x^10 을 왼쪽으로 비트이동 해주고 비트 이동한 만큼 count_a 값에  +1 해준다.
	*/
	int count_a = 0;
	while (((a << count_a) & 0x8000000) != 0x8000000) { // 0x8000000 = 1000 0000 0000 0000 0000 0000 0000 0000 
		count_a++;
	}
	return count_a;
} 
int countd(unsigned int a) {
	/* <전체적인 아이디어>
	이 코드는 주어진 값 a의 최대차수를 구하는 함수이다. 여기서 위의 함수 Count_degree와 다른 점은 
	 Count_degree는  x^32를 기준으로 x^k 까지 차 즉 32-k 값을 구하는 함수이고 
	 countd  함수는 x^k 의 k값을 구하는 함수이다. 

	 그래서 계산방법을 다음과 같이 생각을 했다. 
	 1. a값을 저장하는  tmp값을 선언을 해준다. 
	 2. tmp 값이 0이 될때까지  count++ 해주면서 다음 계산을 해준다. 
	 2-1.	 만약 tmp (tmp & (0x01 << count)) == (0x01 << count) 이면 그 자리의 1을 없애준다. 
	 3. count - 1을 리턴한다. (그 이유는 상수항부터 count++ 을 해주기 때문에)
	*/
	int count = 0;
	unsigned int tmp;

	tmp = a;

	while (tmp != 0) {
		if ((tmp & (0x01 << count)) == (0x01 << count))
		{
			tmp ^= (0x01 << count);
		}
		count++;
	}
	return count - 1;
}

//나누기 연산 함수
unsigned char Divide(unsigned int B,unsigned int A) { // B/A 에서 몫을 구하는 함수
	/* <전체적인 아이디어>
	예를 들어서 0x11b / 0x02라고 해보자. 0x11b = x^8 + x^4 + x^3 + x +1, 0x02 = x

	우리가 나누기 연산을 해주는 과정은 다음과 같이 정리할 수 있다. 
	1. 두 다항식의 최고 차항이 무엇인지 확인한다. (0x11b 의 최고차항 = x^8, 0x02 의 최고차항 = x)
	2. 최고차항의 지수들을 빼준다. (x^(8-1) = x^7)  이 값은 몫이 된다. 
	3. 두 최고차항의 지수승을 뺀 값을 0x02에 곱해준 후 0x11b에 뺄샘을 해준다.
	   (0x11b - 0x02 * x^7 =  (x^8 + x^4 + x^3 + x +1) ^ (x * x^7) =  x^4 + x^3 + x +1)
	이 과정들을 A>B 일때까지 반복한다.
	*/
	int count=1; // while 함수에서 초기값이 필요하므로 0이 아닌 값으로 설정
	unsigned int output = 0; 
	while (count > 0) {
		int count_a = 0;
		int count_b = 0;

		count = countd(B) - countd(A);
		//count = Count_degree(A) - Count_degree(B); // 최고 차항의 차를 구하기 위해 a 와 b의 count 값을 빼준다.
		B = B ^ (A << count); // 나눗셈 방식처럼 최고 차항의 차만큼  A에 곱해주고 이를 XOR 시켜준다. 
		output = output ^ (0x01 << count); // 최고 차항의 차를 몫에 넣어준다.
	}

	output = output & 0xff; //이 함수의 출력값이 char 이기 떄문에 8비트로 만들어준다. 
	return output;
}
unsigned int LongDivision(unsigned int a, unsigned int b, unsigned int *r) { // a>b , r은 나머지 값
	/* <전체적인 아이디어>
	Long division algorithm 을 기반으로 한 코드이며 이는 처음 a와 b의 최대차수 구하는것 이외에
	차수를 계산하지 않고 나눗셈을 구현하는 코드이다.

	맨처음에는 a와 b의 최대차수를 구하고 그의 차를 구한다. 
	그후 만약 최대차수 차가 0이면 같은차수이기 떄문에 몫은 1로 반환한다.

	그렇지 않으면
 	a = a^(b<<count) 를 해주고 output ^= 0x01<<count 해준다.
	a값의 앞의 계수가 1이면 a = a^(b<<count-1) output ^= 0x01<<(count-1) 해준다.  , 계수가 0이면 다음 차수로 건너뛴다. 
	위 계산을 반복해서 a<b 일때 몫을 반환한다.

	https://en.wikipedia.org/wiki/Polynomial_long_division 참고
	*/
	int i;
	int countb = countd(b);
	unsigned int output = 0;

	if (countb == 31) {
		*r = a ^ b;
		return 1;
	}

	for (i = 31; i >= countb; i--) {
		if ((a & (0x01 << i)) == (0x01 << i)) {
			a ^= b << (i-countb);
			output ^= 0x01 << (i-countb);
		}
	}

	*r = a; // call by reference 참고 

	return output;
}

//모듈러 연산 함수
unsigned int Modulo0x11b(unsigned int k) {
	/* <전체적인 아이디어>
	k mod (0x11b) 의 값을 구하는 과정이다.
	1. x^8 이상의 차수의 계수들을 카운트 해준다.
	2, k값에 (0x11b << count) 값을 xor시켜준다.
	3. k값의 최대차수가 8미만이 될때까지 계속 반복한다.
	*/
	unsigned int L;
	int count;

	while (k >= 0x100) {
		count = 0;
		L = k & 0xfffffff00; // 우리가 count 해야될 지수는 x^8 이상이므로 밑에 차수는 버린다.
		while (((L >> count) & 0x100) != 0x100) {
			count++;
		}
		L = L ^ (0x100 << count);
		k = k ^ (0x11b << count); // x^8 ≡ x^4 + x^3 + x + 1 mod (f(x)) 이다. 예를들어서 x^10 = x^8 * x^2 ≡ (x^4 + x^3 + x + 1) * x^2 mod (f(x)) -> 0x1b << 2
	}
	return k;
}

//곱셈 연산 함수
unsigned int Multiplication(unsigned int a, unsigned int b) { //a*b 값을 출력해주는 함수
	/* <전체적인 아이디어>
	이해를 돕기위해 예를 들어서  x^2 + x + 1 = 0x07 과 x + 1 = 0x03 를 생각해보자 
	두 다항식을 곱하는 것을 다음과 같이 생각할수 있다. 
	(x^2 + x + 1) * (x + 1) = (x^2 + x + 1) *x + (x^2 + x + 1) * 1 = (0x07 <<1) ^(0x07 <<0)
	*/

	if (a == 0 || b == 0)
		return 0; // a, b 둘중 하나라도 0 이면 0값을 출력
	if (a == 1)
		return b;
	if (b == 1)
		return a;
	
	int i = 0;

	unsigned int output = 0;


	while (a != 0) {
		if (((a >> i) & 0x01) == 1) {
			output ^= b << i;
			a ^= 0x01 << i;
		}
		i++;
	}
	//a와 b의 차수를 따로 계산하지 않아도 연산 가능;;;

	return output;
}
unsigned char new_multiplication(unsigned char x, unsigned char y) {
	/* <전체적인 아이디어>
	이 함수를 만든 이유는 다항식의 곱셈 연산에서 차수를 구하지 않고 계산하는 방식을 생각해 보고자 만든 함수이다.
	두 다항식 a(x) = a_7x^7 + ... + a_1x + a_0 와 b(x) = b_7x^7 + ... + b_1x + b_0의 곱을 구하는 것을 다음과 같이 생각했다. 
	a(x) * b(x) 를 그냥 전개시키면 다음과 같다.
	let a(x) * b(x) = c(x) 
	c(x) = (a_7b_7)x^14 +  (a_7b_6^ a_6b_7)x^13 ...
	위 식을 그대로 구현 한 것이다. 

	그리고 모듈러 연산도 구현을 했는데 이를 행렬의 곱으로 표현을 했다. 
	우리가 계산한 식 c(x) 는 14차 다항식이다. 그래서 x^8 = x^4 + x^3 + x + 1을 대입해서  14차 다항식을 8차 미만 다항식으로 만들어서
	행렬로 나타내면 다음과 같은 행렬 식이 나온다.

	{1,0,0,0,0,0,0,0,1,0,0,0,1,1,0},
	{0,1,0,0,0,0,0,0,1,1,0,0,1,0,1},
	{0,0,1,0,0,0,0,0,0,1,1,0,0,1,0},
	{0,0,0,1,0,0,0,0,1,0,1,1,1,1,1},
	{0,0,0,0,1,0,0,0,1,1,0,1,0,0,1},
	{0,0,0,0,0,1,0,0,0,1,1,0,1,0,0},
	{0,0,0,0,0,0,1,0,0,0,1,1,0,1,0},
	{0,0,0,0,0,0,0,1,0,0,0,1,1,0,1}

	그래서 이 행렬식을 곱하는데 이때 우리는 다항식을 행렬로 표현할때 오름차순으로 생각을 할것이다. 
	*/
	unsigned char temp[15] = { 0, };

	unsigned char matrix[15] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a };
	unsigned char output = 0;
	int i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j <= i; j++) {
			temp[i] ^= ((x >> j) & 0x01) & ((y >> (i - j)) & 0x01);
		}
		if (i < 7) {
			for (j = 0; j <= i; j++) {
				temp[14 - i] ^= ((x >> (7 - j)) & 0x01) & ((y >> (7 - i + j)) & 0x01);
			}
		}
	}//곱셈 연산을 하는 과정

	for (i = 0; i < 15; i++) {
		if (temp[i] == 1)
			output ^= matrix[i];
	}//모듈러 연산을 하는 과정
	return output;
}

// 역원 구하는 함수
unsigned int ExtendEuclideanFunction(unsigned int C) {
	/* <전체적인 아이디어>
	확장 유클리드 알고리듬 정의 그대로를 구현 대신 각 연산마다 필요한건 각 값마다 두개 값만 필요하기 때문에
	변수들 k[2] 이런식으로만 설정함
	*/
	unsigned int R[2] = { 0x00, };
	unsigned int U[2] = { 0x00, };
	unsigned int W[2] = { 0x00, };
	unsigned int Q;

	if (C == 0x00)
		return C; 

	R[0] = 0x11b; R[1] = C;
	U[0] = 0x00; U[1] = 0x01;
	W[0] = 0x01; W[1] = 0x00;

	unsigned int tmp = 0;
	while (R[1] != 1) {
		Q = LongDivision(R[0], R[1],&tmp);
		R[0] = R[1]; R[1] = tmp;
		//R[0] = R[1]; R[1] = tmp;
		tmp = U[1]; U[1] = U[0] ^ Multiplication(tmp, Q); U[0] = tmp;

		tmp = W[1]; W[1] = W[0] ^ Multiplication(tmp, Q); W[0] = tmp;
		/*printf("R[0] = %02x, R[1] = %02x\n", R[0], R[1]);
		printf("U[0] = %02x, U[1] = %02x\n", U[0], U[1]);
		printf("W[0] = %02x, W[1] = %02x\n", W[0], W[1]);
		printf("Q = %02x\n", Q);*/
	}

	return U[1];
}
unsigned int inverseFunction2(unsigned int k) {
	/* <전체적인 아이디어>
	오일러 정리를 이용했다. 
	오일러 정리란 a(x) 가 F[x]/f(x)에 존재할 때 a(x)^{Φ(f(x))}≡ 1 mod (f(x)) 이다. 
	여기서 Φ(f(x)) = |{ a(x) !=0 \in F[x]/f(x) such that GCD(a(x) , f(x)) = 1}| 이다. 
	AES 에서는 f(x) = x^8 + x^4 + x^3 + x + 1 인 8차 기약다항식을 사용하므로 Φ(f(x)) = 2^8 - 1이다.
	a(x)^{2^8-1} ≡ 1 mod (f(x)) 이므로 a(x)의 역원은  a(x) ^{2^8 - 2} mod (f(x)) 이다.

	여기서 계산을 할때 32바이트를 넘어가지 않게 하기 위해서 k^n 이 x^8 보다 크거나 같은 다항식일때 mod (f(x)) 해주면서 연산을 진행한다. 
	*/
	int i;
	unsigned int L = k;
	for (i = 2; i < 255; i++) {
		k = Multiplication(L, k);
		k = Modulo0x11b(k);
	}
	return k;
}
unsigned int InverseFunction3(unsigned int k) {
	/*<전체적인 아이디어>
	a^{2^8-2} mod 0x11b 값을 구하는 과정을 간략하게 한것이다. 
	a^254 = a^127 * a^127
	a^127 = a^63 * a^64 (a^64 = a^32 * a^32)
	a^63 = a^31 * a^32 (a^32 = a^16 * a^16)
	a^31 = a^15 * a^16 (a^16 = a^8 * a^8)
	a^15 = a^7 * a^8 (a^8 = a^4*a^4)
	a^7 = a^3 ^ a^4 (a^4 = a^2 * a^2)
	a^3 = a * a^2 (a^2 = a * a)
	*/
	unsigned int tmp[2] = { k, k };
	int i;

	for (i = 0; i < 7; i++) {
		if (i == 6) {
			tmp[1] = Multiplication(tmp[1], tmp[1]);
			tmp[1] = Modulo0x11b(tmp[1]);
			return tmp[1];
		}
		tmp[0] = Multiplication(tmp[0], tmp[0]);
		tmp[0] = Modulo0x11b(tmp[0]);
		
		tmp[1] = Multiplication(tmp[1], tmp[0]);
		tmp[1] = Modulo0x11b(tmp[1]);
	}
	return 0;
}
//아핀변환 함수
unsigned char AffineTransformation(unsigned char a) {
	/* aes 규격서를 토대로 행렬의 곱을 이용해서 만든것이다. */
	int i, j;
	unsigned char matrix[8][8] = {
	 {1,0,0,0,1,1,1,1}
	,{1,1,0,0,0,1,1,1}
	,{1,1,1,0,0,0,1,1}
	,{1,1,1,1,0,0,0,1}
	,{1,1,1,1,1,0,0,0}
	,{0,1,1,1,1,1,0,0}
	,{0,0,1,1,1,1,1,0}
	,{0,0,0,1,1,1,1,1} };

	unsigned char c[8] = { 1,1,0,0,0,1,1,0 };
	unsigned char b[8] = { 0, };
	unsigned char k;
	unsigned char output = 0;

	for (i = 0; i < 8; i++) {
		b[i] = (a >> i) & 0x01;
	}
	for (i = 0; i < 8; i++) {
		k = matrix[i][0] * b[0];
		for (j = 1; j < 8; j++) {
			k ^= matrix[i][j] * b[j];
		}
		output ^= (k^c[i]) << i;
	}

	return output;
}
unsigned char InverseAffineTransformation(unsigned int a) {
	// aes 규격서를 토대로 행렬의 곱을 이용하여 만든것이다. 
	int i, j;
	unsigned char matrix[8][8] = {
	{0,0,1,0,0,1,0,1},
	{1,0,0,1,0,0,1,0},
	{0,1,0,0,1,0,0,1},
	{1,0,1,0,0,1,0,0},
	{0,1,0,1,0,0,1,0},
	{0,0,1,0,1,0,0,1},
	{1,0,0,1,0,1,0,0},
	{0,1,0,0,1,0,1,0}
	};

	unsigned char c[8] = { 1,0,1,0,0,0,0,0 };
	unsigned char b[8] = { 0, };
	unsigned char k;
	unsigned char output = 0;

	for (i = 0; i < 8; i++) {
		b[i] = (a >> i) & 0x01;
	}
	for (i = 0; i < 8; i++) {
		k = matrix[i][0] * b[0];
		for (j = 1; j < 8; j++) {
			k ^= matrix[i][j] * b[j];
		}
		output ^= (k^c[i]) << i;
	}

	return output;
}

/*
이제 볼 내용은 x^n을 구하는 세가지 알고리듬을 공부를 할것이다. 이 내용에 대해서 공부하기전 우리는 다음과 같이 약속하고 넘어가자.

						n = n_{L-1}n_{L-2}...n_1n_0 : n 값은 L비트로 나타낸다.
						X^n = X^{sum_{j=0}^{L-1} n_j*2^j} = X^{n_{L-1}2^{L-1}} * ... X^{n_0}

*/
//left to right
unsigned char LTR(unsigned char x, unsigned int n) {
	/*<전체적인 아이디어>
	이 코드는 left to right binary method 이용해서 x의 n승을 구하는 코드이다. 
	여기서 n승을 비트로 표현해서 계산을 할것이다. 
	예시를 통해서 아이디어를 설명하자면 다음과 같다. 
	우리가 x^254 = x^{2^7 + 2^6 + 2^5 + 2^4 + 2^3 + 2^2 + 2^1} = x^{1111 1010} 를 계산하자고 하자
	그러면 left to right binary method 는 이와 같이 생각하는 방식이다
	( ( ( ( ( ( ( ( ( (x^2) x)^2) x)^2) x)^2) x)^2) (x^0))^2) x)^2 이렇게 생각한후 괄호 맨 안쪽부터 계산하는 방식이다. 
	이를 수도코드로 일반화를 시키면 다음과 같다.

	-------------------------------------------------------------------------
	n 을 비트로 {n_ln{l-1}...n_1n0} 로 표현하자 
	Input: x and n 
	Output: x^n 
	1: t <- 1 
	2: for i <- l - 1 down to 0 do 
	3: t <- t^2 
	4: t <- t*x^{n_i} 
	5: end for 
	6: return t
	-----------------------------------------------------------------------------
	*/
	int i,count;
	unsigned char temp[2];

	count = countd(n);
	temp[0] = x;

	for (i = count-1; i >= 0; i--){
		temp[1] = new_multiplication(temp[0], temp[0]);
		if (((n >> i) & 0x01) == 0x01)
			temp[0] = new_multiplication(temp[1], x);
		else
			temp[0] = temp[1];
	}

	return temp[1];
}

//right to left
unsigned char RTL(unsigned char x, unsigned int n) {
	/*<전체적인 아이디어>
	이 코드는 a^n 을 구하기 위해 Right-to-Left Binary Method 를 이용한 c코드이다. 
	n 을 비트로 {n_ln{l-1}...n_1n0} 로 표현할때 다음의 관계식을 이용할 것이다. 

	a^{sum_{j=0}^{l-1} n_j2^j} = {a^{2^0}}^n_0 * {{a^{2^0}}^2}^n_1 * {{{a^{2^0}}^2}^2}^n_2 * ... 

	이를 계산하는 수도코드는 다음과 같다. 
	---------------------------------------------------------------------------
	Input : x and n
	output : x^n 
	1 : t_0 <- 1
	2 : t_1 <- x
	3 : for i <- 0 to l - 1 do
	4 :		if n_i == 1 then
	5 :			t_0 <- t_0 * t_1
	6 :		end if
	7 :		t_1 <- {t_1}^2
	8 : end for
	9 : return t_0
	----------------------------------------------------------------------------
	*/
	unsigned char t[2];
	int i, count;

	count = countd(n); // l값 구해줌
	t[0] = 1; t[1] = x;

	for (i = 0; i <= count; i++) {
		if (((n >> i) & 0x01) == 0x01)
			t[0] = new_multiplication(t[0], t[1]);

		t[1] = new_multiplication(t[1], t[1]);
	}
	return t[0];
}

//Multiply and Squaring
unsigned char MAS(unsigned char x, unsigned int n) {
	/* <전체적인 아이디어> 
	기본적인 이론은 다음과 같다. 
	n값이 있을때 다음과 같이 계산을 할것이다. 	
	1. n이 홀수일때, x^n = x ( x^2 )^{(n-1)/2}
	2. n이 짝수일때, x^n = (x^2)^{n/2}

	예시로 x^13을 계산을 해보자. 여기서 13 = 1101 로 나타낼수있다. 

	First, initialize the result to 1:
	r <- 1 ( = x^0 ) 
	Step 1
		r <- r^2 ( = x^0 )
		bit 1 = 1, so compute
		r <- r ⋅ x ( = x^1 ) 
	Step 2
		r <- r^2 ( = x^2 ) 
		; bit 2 = 1, so compute
		r <- r ⋅ x ( = x^3 ) 
	Step 3
		r <- r^2 ( = x^6 ) 
		bit 3 = 0, so we are done with this step (equivalently, this corresponds to r <- r ⋅ x^0 ( = x^6 ) )
	Step 4
		r <- r^2 ( = x ^2 ) 
		bit 4 = 1, so compute
		r <- r ⋅ x ( = x^13 ) 

	이를 수도코드로 나타내면 다음과 같다.
	----------------------------------------------------
	Input : x and n
	output : x^n
	1 : t_0 <- 1
	2 : t_1 <- x
	3 : for i <- l-1 down to 0 do
	4 :		t_{l-n_i} <- t_0 * t_1
	5 :		t_{n_i} <- (t_{n_i})^2
	6 : end for
	7 : return t_0
	----------------------------------------------------
	*/
	int count, i; 
	unsigned char t[2] = { 1, x };

	count = countd(n);

	for (i = count; i >= 0; i--) {
		t[1 ^ ((n >> i) & 0x01)] = new_multiplication(t[0], t[1]);
		t[(n >> i) & 0x01] = new_multiplication(t[(n >> i) & 0x01], t[(n >> i) & 0x01]);
	}

	return t[0];
}

int main() {
	unsigned int A;
	unsigned char B;
	/*printf("A의 값을 입력하세요 : ");
	scanf_s("%02x", &A, 8);

	printf("%02x의 역원은 %02x입니다.\n",A, inverseFunction(A));
	B = inverseFunction(A) & 0xff;
	printf("%02x의 s-box변환값은 %02x입니다.\n",A, Matrix(B));

	*/

	printf("          S-BOX\n");
	for (A = 0x00; A <= 0xff; A++) {
		B = ExtendEuclideanFunction(A) & 0xff;
		printf("%02x ", AffineTransformation(B));
		if ((A & 0x0f) == 0x0f)
			printf("\n");
	}
	
	printf("\n \n");
	
	printf("          Inverse S-BOX\n");
	for (A = 0x00; A <= 0xff; A++) {
		B = InverseAffineTransformation(A) & 0xff;
		printf("%02x ", ExtendEuclideanFunction(B)&0xff);
		if ((A & 0x0f) == 0x0f)
			printf("\n");
	}

	printf("\n\n 이부분은 역원값이 같은지 비교하기 위해서 만들었습니다. \n");
	for (int k = 0x01; k <= 0xff; k++) {
		printf("%02x의 역원 : %02x = %02x = %02x = %02x = %02x = %02x",k, ExtendEuclideanFunction(k), inverseFunction2(k), InverseFunction3(k), LTR(k, 254),RTL(k,254),MAS(k,254));
		if (ExtendEuclideanFunction(k) == MAS(k, 254))
			printf(" 역원값 잘 만들었어요 ^_^ \n");
	}
	/*for (unsigned int i = 0; i < 0x100; i++) {
		for (unsigned int j = 0; j < 0x100; j++) {
			unsigned int k = Multiplication(i, j);
			if (Modulo0x11b(k) == new_multiplication(i, j))
				printf("%02x x %02x = 잘만듬 \n", j, i);
			else
				printf("%d x %d = ㅗ \n", j, i);
			
		}
	}*/

	return 0;
}
