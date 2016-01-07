#ifndef color_container_h_
#define color_container_h_

#define MATERIAL_PEARL 0
#define MATERIAL_PEWTER 1
#define MATERIAL_SILVER 2
#define MATERIAL_CHROME 3
#define MATERIAL_POLISHED_SILVER 4
#define MATERIAL_OBSIDIAN 5
#define MATERIAL_BLACK_PLASTIC 6
#define MATERIAL_JADE 7
#define MATERIAL_TURQUOISE 8
#define MATERIAL_COPPER 9
#define MATERIAL_EMERALD 10
#define MATERIAL_LIGHT_BLUE 11
#define MATERIAL_GOLD	12
#define MATERIAL_POLISHED_GOLD 13
#define MATERIAL_POLISHED_BRONZE 14
#define MATERIAL_BRONZE 15
#define MATERIAL_BRASS 16
#define MATERIAL_GRASS 17
#define MATERIAL_POLISHED_COPPER 18
#define MATERIAL_RUDY 19
#define MATERIAL_USER_DEFINED 20

struct ColorContainer {
 public:
  ColorContainer();
  ~ColorContainer();
  
  void ChangeMaterial(int material, bool front);
  bool EditColor(int type);
  void EnableColorMaterial();
  void DisableColorMaterial();
  void HSVtoRGB(const float &weight,
    float *x,
    float *y,
    float *z);
  void SetColor(const float &val);
  
  int surface_material;
  int	feature_material;
  int	corres_material;

	/************************************************************************/
	/* Colors
	*/
	/************************************************************************/
	float back_color[3];
	float	mesh_color[3];
	float	edge_color[3];
	float	vertex_color[3];
	float	direc_color[3];

  int code_background;
};

#endif