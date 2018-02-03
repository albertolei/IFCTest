#include"ifcengine.h"
#include "unit.h"

struct STRUCT__SIUNIT {
	int_t							type;
	wchar_t							* unitType;

	wchar_t							* prefix;
	wchar_t							* name;

	STRUCT__SIUNIT					* next;
};
enum ENUM_TREE_ITEM_SELECT_STATE {
	TI_CHECKED = 1,
	TI_UNCHECKED = 3,
	TI_PARTLY_CHECKED = 2,
	TI_NONE = 5
};
enum ENUM_TREE_ITEM_TYPES {
	TREE_ITEM_CONTAINS = 1,
	TREE_ITEM_NOTREFERENCED,
	TREE_ITEM_DECOMPOSEDBY,
	TREE_ITEM_GROUPEDBY,
	TREE_ITEM_GROUPS,
	TREE_ITEM_SPACEBOUNDARIES,
	TREE_ITEM_IFCINSTANCE,
	TREE_ITEM_IFCENTITY,
	TREE_ITEM_GEOMETRY,
	TREE_ITEM_PROPERTIES,
	TREE_ITEM_PROPERTY,
	TREE_ITEM_PROPERTYSET,
	TREE_ITEM_TEXT
};
struct _TREEITEM;
typedef struct _TREEITEM *HTREEITEM;
struct STRUCT_TREE_ITEM {
	ENUM_TREE_ITEM_TYPES			type;

	HTREEITEM						hTreeItem;
	wchar_t							* nameBuffer;

	STRUCT_TREE_ITEM				* parent;
	STRUCT_TREE_ITEM				* child;
	STRUCT_TREE_ITEM				* next;
};
struct STRUCT_TREE_ITEM_SELECTABLE : STRUCT_TREE_ITEM 
{
	ENUM_TREE_ITEM_SELECT_STATE		selectState;
};
typedef struct VECTOR3 {
	double							x;
	double							y;
	double							z;
}	VECTOR3;
struct STRUCT_COLOR {
	float							R;
	float							G;
	float							B;
	float							A;
};
struct STRUCT_MATERIAL {
	bool							active;

	STRUCT_COLOR					ambient;
	STRUCT_COLOR					diffuse;
	STRUCT_COLOR					specular;
	STRUCT_COLOR					emissive;

	double							transparency;
	double							shininess;

	void							* MTRL;

	STRUCT_MATERIAL					* next;
	STRUCT_MATERIAL					* prev;
};
struct STRUCT_MATERIALS {
	//	int64_t							indexArrayOffset;
	//	int64_t							indexArrayPrimitives;

	//	int64_t							indexOffsetForFaces;

	int64_t							__indexOffsetForFaces;
	int64_t							__indexArrayOffset;

	int64_t							__noPrimitivesForFaces;
	int64_t							__indexBufferSize;

	STRUCT_MATERIAL					* material;

	STRUCT_MATERIALS				* next;
};
struct STRUCT__IFC__OBJECT;
struct STRUCT_TREE_ITEM_IFCINSTANCE : STRUCT_TREE_ITEM_SELECTABLE {
	int_t							ifcModel;
	int_t							ifcInstance;
	STRUCT__IFC__OBJECT				*ifcObject;
};
struct STRUCT__IFC__OBJECT {
	int_t							ifcInstance;
	int_t							ifcEntity;

	ENUM_TREE_ITEM_SELECT_STATE		selectState;

	wchar_t							* entityName;
	bool							hide;
	int_t							segmentationParts;
	STRUCT_TREE_ITEM_IFCINSTANCE	* treeItemModel;
	STRUCT_TREE_ITEM_IFCINSTANCE	* treeItemSpaceBoundary;
	STRUCT_TREE_ITEM_IFCINSTANCE	* treeItemNonReferenced;

	STRUCT_MATERIALS				* materials;
	STRUCT__IFC__OBJECT				* next;

	VECTOR3							vecMin;
	VECTOR3							vecMax;

	int_t							noVertices;//顶点个数
	float							* vertices;//每个顶点由6个数组成，存储在此

	int_t							vertexOffset;

	int_t							noPrimitivesForPoints;
	int32_t							* indicesForPoints;
	int_t							indexOffsetForPoints;

	int_t							noPrimitivesForLines;
	int32_t							* indicesForLines;
	int_t							indexOffsetForLines;	

	int_t							noPrimitivesForFaces;
	int32_t							* indicesForFaces;
	int_t							indexOffsetForFaces;

	int_t							noPrimitivesForWireFrame;
	int32_t							* indicesForLinesWireFrame;
	int_t							indexOffsetForWireFrame;
};
struct STRUCT_TREE_ITEM_TEXT : STRUCT_TREE_ITEM {
	bool allocated;
};
#ifndef D3DCOLORVALUE_DEFINED
typedef struct _D3DCOLORVALUE {
	float r;
	float g;
	float b;
	float a;
} D3DCOLORVALUE;

#define D3DCOLORVALUE_DEFINED
#endif

typedef struct _D3DMATERIAL9 {
	D3DCOLORVALUE   Diffuse;        /* Diffuse color RGBA */
	D3DCOLORVALUE   Ambient;        /* Ambient color RGB */
	D3DCOLORVALUE   Specular;       /* Specular 'shininess' */
	D3DCOLORVALUE   Emissive;       /* Emissive color RGB */
	float           Power;          /* Sharpness if specular highlight */
} D3DMATERIAL9;
struct STRUCT_TREE_ITEM_GEOMETRY : STRUCT_TREE_ITEM_SELECTABLE {};
struct STRUCT_TREE_ITEM_PROPERTIES : STRUCT_TREE_ITEM {};
struct STRUCT_TREE_ITEM_DECOMPOSEDBY : STRUCT_TREE_ITEM_SELECTABLE {};
struct STRUCT_TREE_ITEM_CONTAINS : STRUCT_TREE_ITEM_SELECTABLE {};
struct STRUCT_TREE_ITEM_SPACEBOUNDARIES : STRUCT_TREE_ITEM_SELECTABLE {};
struct STRUCT_TREE_ITEM_NOTREFERENCED : STRUCT_TREE_ITEM_SELECTABLE {};
struct STRUCT_TREE_ITEM_IFCENTITY : STRUCT_TREE_ITEM_SELECTABLE 
{
	int_t							ifcModel;
	int_t							ifcEntity;
};
struct STRUCT_TREE_ITEM_GROUPEDBY : STRUCT_TREE_ITEM_SELECTABLE {};
struct STRUCT_TREE_ITEM_GROUPS : STRUCT_TREE_ITEM_SELECTABLE {};

struct STRUCT__PROPERTY {
	int_t							structType;
	int_t							ifcInstance;

	wchar_t							* name;
	wchar_t							* description;

	wchar_t							* nominalValue;
	wchar_t							* lengthValue;
	wchar_t							* areaValue;
	wchar_t							* volumeValue;
	wchar_t							* countValue;
	wchar_t							* weigthValue;
	wchar_t							* timeValue;
	wchar_t							* unit;

	HTREEITEM						hTreeItem;

	wchar_t							* nameBuffer;

	STRUCT__PROPERTY				* next;
};
struct STRUCT__PROPERTY__SET {
	int_t							structType;
	int_t							ifcInstance;

	wchar_t							* name;
	wchar_t							* description;

	STRUCT__PROPERTY				* properties;

	HTREEITEM						hTreeItem;

	wchar_t							* nameBuffer;

	STRUCT__PROPERTY__SET			* next;
};