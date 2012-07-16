#include "UBCubicPolynomial.h"

UBCubicPolynomial::UBCubicPolynomial(){

}

UBCubicPolynomial::UBCubicPolynomial(float a, float b, float c, float d){
	mA = a;
	mB = b;
	mC = c;
	mD = d;
}

UBCubicPolynomial::~UBCubicPolynomial(){

}

float UBCubicPolynomial::eval(float u){
	return (((mD*u) + mC)*u + mB)*u + mA;
 }
