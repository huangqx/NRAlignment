#ifndef moment_h_
#define moment_h_

#include "linear_algebra.h"
#include "linear_algebra_templcode.h"

class Moment3d {   
 public:
	// Translation is the vector
	// followed by a dim x dim matrix
	double	order2[3][3];
	double	order1[3];
	double	order0;
 public:
	// ---- constructors
	inline Moment3d();
	
	//  ----  operators +, -, *, /
	inline Moment3d operator+(const Moment3d &op) const;
	inline Moment3d operator-(const Moment3d &op) const;
	inline Moment3d operator-() const;
	inline Moment3d operator*(const double &s) const;

	inline Moment3d operator/(const double &s) const;

	//  ---- operators +=, -=, *=, /=
	inline Moment3d operator+=(const Moment3d &op);
	inline Moment3d operator-=(const Moment3d &op);
	inline Moment3d operator*=(const double &op);
	inline Moment3d operator/=(const double &op);
	inline Moment3d operator=(const Moment3d &op);

  // Update
  inline void InsertAPoint(const Vector3f &pos, const float &w);
  inline void InsertAPoint(const double *pos, double w);

	inline double GetSize();	
	inline void  SetZero();
};


#include "moment_templcode.h"

#endif
