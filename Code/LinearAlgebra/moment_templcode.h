#ifndef moment_templcode_h_
#define moment_templcode_h_

#include <math.h>
#include <string>

#include "moment.h"

using namespace std;

/************************************************************************/
/* moment                                                             */
/************************************************************************/

inline Moment3d Moment3d::operator+(const Moment3d &op) const {
	Moment3d result;
	result.order0 = order0 + op.order0;
	for (int i = 0; i < 3; i++) {
		result.order1[i] = order1[i] + op.order1[i];
		for (int j = 0; j < 3; j++)
			result.order2[i][j] = order2[i][j] + op.order2[i][j];
	}
	return result;
}


inline Moment3d Moment3d::operator-(const Moment3d &op) const {
	Moment3d result;
	result.order0 = order0 - op.order0;
	for (int i = 0; i < 3; i++) {
		result.order1[i] = order1[i] - op.order1[i];
		for (int j = 0; j < 3; j++)
			result.order2[i][j] = order2[i][j] - op.order2[i][j];
	}
	return result;
}

inline Moment3d Moment3d::operator-() const {
	Moment3d result;
	result.order0 = -order0;
	for (int i = 0; i < 3; i++) {
		result.order1[i] = -order1[i];
		for (int j = 0; j < 3; j++)
			result.order2[i][j] = -order2[i][j];
	}
	return result;
}

inline Moment3d Moment3d::operator*(const double &s) const {
	Moment3d result;
	result.order0 = order0*s;
	for (int i = 0; i < 3; i++) {
		result.order1[i] = order1[i] * s;
		for (int j = 0; j < 3; j++)
			result.order2[i][j] = order2[i][j] * s;
	}
	return result;
}

inline Moment3d Moment3d::operator/(const double &s) const {
	Moment3d result;
	result.order0 = order0/s;
	for (int i = 0; i < 3; i++) {
		result.order1[i] = order1[i] / s;
		for (int j = 0; j < 3; j++)
			result.order2[i][j] = order2[i][j] / s;
	}
	return result;
}

inline Moment3d Moment3d::operator+=(const Moment3d &op) {
	order0 += op.order0;
	for (int i = 0; i < 3; i++) {
		order1[i] += op.order1[i];
		for (int j = 0; j < 3; j++)
			order2[i][j] += op.order2[i][j];
	}
	return *this;
}

inline Moment3d Moment3d::operator-=(const Moment3d &op) {
	order0 -= op.order0;
	for (int i = 0; i < 3; i++) {
		order1[i] -= op.order1[i];
		for (int j = 0; j < 3; j++)
			order2[i][j] -= op.order2[i][j];
	}
	return *this;
}

inline Moment3d Moment3d::operator*=(const double &op) {
	order0 *= op;
	for (int i = 0; i < 3; i++) {
		order1[i] *= op;
		for (int j = 0; j < 3; j++)
			order2[i][j] *= op;
	}
	return *this;
}

inline Moment3d Moment3d::operator/=(const double &op) {
	order0 /= op;
	for (int i = 0; i < 3; i++) {
		order1[i] /= op;
		for (int j = 0; j < 3; j++)
			order2[i][j] /= op;
	}
	return *this;
}

inline Moment3d Moment3d::operator=(const Moment3d &op) {
	order0 = op.order0;
	for (int i = 0; i < 3; i++) {
		order1[i] = op.order1[i];
		for (int j = 0; j < 3; j++)
			order2[i][j] = op.order2[i][j];
	}
	return *this;
}

inline Moment3d::Moment3d() {
	SetZero();
}

inline double Moment3d::GetSize() {
	double	order2_n[3][3], order1_n[3];
	for (int i = 0; i < 3; i++) {
		order1_n[i] = order1[i]/order0;
		for (int j = 0; j < 3; j++)
			order2_n[i][j] = order2[i][j]/order0;
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			order2_n[i][j] -= order1_n[i]*order1_n[j];
	}
	return sqrt(order2_n[0][0]+order2_n[1][1]+order2_n[2][2]);
}

inline void Moment3d::SetZero() {
	order0 = 0.0;
	for (int i = 0; i < 3; i++) {
		order1[i] = 0.0;
		for (int j = 0; j < 3; j++)
			order2[i][j] = 0.0;
	}
}

inline void Moment3d::InsertAPoint(const Vector3f &pos, const float &w) {
  order0 += w;
  for (int i = 0; i < 3; i++) {
    order1[i] += pos[i] * w;
    for (int j = 0; j < 3; j++) {
      order2[i][j] += pos[i] * pos[j] * w;
    }
  }
}


inline void Moment3d::InsertAPoint(const double *pos, double w) {
  order0 += w;
  for (int i = 0; i < 3; i++) {
    order1[i] += pos[i] * w;
    for (int j = 0; j < 3; j++) {
      order2[i][j] += pos[i] * pos[j] * w;
    }
  }
}

#endif
