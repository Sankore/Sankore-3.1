#ifndef UBCUBICPOLYNOMIAL_H
#define UBCUBICPOLYNOMIAL_H

class UBCubicPolynomial{
public:
	UBCubicPolynomial();
	UBCubicPolynomial(float a, float b, float c, float d);
	~UBCubicPolynomial();
	float eval(float u);

private:
	float mA;
	float mB;
	float mC;
	float mD;
};

#endif	// UBCUBICPOLYNOMIAL_H
