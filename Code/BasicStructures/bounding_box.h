#ifndef bounding_box_h_
#define bounding_box_h_

#include "linear_algebra_templcode.h"
#include "linear_algebra.h"

struct BoundingBox {
 public:
	BoundingBox();
	~BoundingBox();

	/************************************************************************/
	/* IO Operations
	*/
	/************************************************************************/
	void Read(FILE *file_ptr);
	void Write(FILE *file_ptr);

	// Update the size of the bounding box 
  // when adding a new bounding box
	void Insert_A_Box(BoundingBox* newbox);
	
	// Update the size of the bounding box 
  // when adding a new point
	void Insert_A_Point(float* m_vPos);

	void Insert_A_Point(const Vector3f	&p);
	
	//Insert the feature values of a point
	void Insert_A_Feature(float* signatures);

	//Initialize 
	void Initialize();

	//The High Corner of this bounding box
	Vector3d	upper_corner;
	
	//The Low Corner of this bounding box
	Vector3d	lower_corner;
	
	//The size of this bounding box
	Vector3d	size;

	//The middle of thie bounding box
	Vector3d	center_point;
};
#endif