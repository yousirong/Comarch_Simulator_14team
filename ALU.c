/*ALU함수 부분이다.
1. logicalOperation() = 비트 연산자 수행하는 함수
2. addSubtract() = 덧셈,뺄셈 수행하는 함수이다.
3. shiftOperation() = 쉬프트 연산 함수이다.
4. checkZero() = 0인지 확인하는 함수이다.
5. checkSetLess() = x보다 작은지 판별하는 함수이다.*/
int logicOperation(int X, int Y, int C) {
	if (C < 0 || C > 3) {
		printf("error in logic operation\n");
		exit(1);
	}
	if (C == 0) {
		// AND
		return X & Y;
	}
	else if (C == 1) {
		// OR
		return X | Y;
	}
	else if (C == 2) {
		// XOR
		return X ^ Y;
	}
	else {
		// NOR
		return ~(X | Y);
	}
}

int addSubtract(int X, int Y, int C) {
	int ret;
	if (C < 0 || C > 1) {
		printf("error in add/subtract operation\n");
		exit(1);
	}
	if (C == 0) {
		// add
		ret = X + Y;
	}
	else {
		// subtract
		ret = X - Y;
	}
	return ret;
}

// V is 5 bit shift amount
int shiftOperation(int V, int Y, int C) {
	int ret;
	if (C < 0 || C > 3) {
		printf("error in shift operation\n");
		exit(1);
	}
	if (C == 0) {
		// No shift : 그대로 반환
		ret = V;
	}
	else if (C == 1) {
		// Logical left
		ret = V << Y;
	}
	else if (C == 2) {
		// Logical right
		ret = V >> Y;
	}
	else {
		// Arith right
		ret = V >> Y;
	}
	return ret;
}

// 이함수는 add 또는 subtract 수행 시만
// 사용하여 Z값을 설정한다.
int checkZero(int S) {
	int ret = 0;
	// check if S is zero,
	// and return 1 if it is zero
	// else return 0
	if (S == 0) {
		ret = 1;
	}
	return ret;
}

int checkSetLess(int X, int Y) {
	int ret;

	// check if X < Y,
	// and return 1 if it is true
	// else return 0
	if (Y > X) {
		ret = 1;
	}
	else {
		ret = 0;
	}
	return ret;
}

int ALU(int X, int Y, int C, int* Z) {
	int c32, c10;
	int ret;

	c32 = (C >> 2) & 3;
	c10 = C & 3;
	if (c32 == 0) {
		//shift
		ret = shiftOperation(X, Y, c10);
	}
	else if (c32 == 1) {
		// set less
		ret = checkSetLess(X, Y);
	}
	else if (c32 == 2) {
		// addsubtract
		ret = addSubtract(X, Y, c10);
		*Z = checkZero(ret);
	}
	else {
		// logic
		ret = logicOperation(X, Y, c10);
	}
	return ret;
}