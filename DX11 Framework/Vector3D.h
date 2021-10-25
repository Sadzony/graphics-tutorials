#pragma once
#include<iostream>
#include<math.h>
#include<assert.h>
using namespace std;
class Vector3D
{
public:
	float x, y, z;
	//Constructors
	Vector3D(); //default constructor
	Vector3D(float x1, float y1, float z1); //constructor with values
	Vector3D(const Vector3D& vec); //copy constructor

	//Artithmetic Operators
	Vector3D operator+(const Vector3D& vec); //vector addition, subtraction etc
	Vector3D& operator+=(const Vector3D& vec);
	Vector3D operator-(const Vector3D& vec);
	Vector3D& operator-=(const Vector3D& vec);
	Vector3D operator*(float value);
	Vector3D& operator*=(float value);
	Vector3D operator/(float value);
	Vector3D& operator/=(float value);
	Vector3D& operator=(const Vector3D& vec);

	//Vector Operations
	float dot_product(const Vector3D& vec) const; //dot product of 2 vectors
	Vector3D cross_product(const Vector3D& vec) const; //cross product of 2 vectors
	Vector3D normalised(); //return normalized version of this vector

	//Scalar Operations
	float square() const; //gives a square of the vector
	float distance(const Vector3D& vec) const; //distance between two vectors
	float magnitude() const; //magnitude of this vector

	//Get operations
	float get_x() const;
	float get_y() const;
	float get_z() const;
	void disp() const; //display value of vectors
};

