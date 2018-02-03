#include<iostream>
#include<fstream>
#include <list>
#include "engine.h"
#include "ifcengine.h"
#include "gene.h"
#include<gl/glut.h>
#include <unordered_map>
#include <cstdlib>

using namespace std;
//函数声明
bool contains(wchar_t *txtI, wchar_t *txtII);
STRUCT__IFC__OBJECT	* CreateIfcObject(int_t ifcEntity, int_t ifcInstance, wchar_t * entityName, bool hide, int_t segmentationParts);
STRUCT__IFC__OBJECT** queryIfcObjects(int_t ifcModel, STRUCT__IFC__OBJECT	**firstFreeIfcObject, wchar_t *entityName, bool hide, int_t	segmentationParts);
STRUCT__IFC__OBJECT** GetChildrenRecursively(int_t ifcModel, int_t ifcParentEntity, STRUCT__IFC__OBJECT **firstFreeIfcObject, bool hide, int_t segmentationParts);
STRUCT_TREE_ITEM** CreateTreeItem_ifcSpaceBoundaryDecomposedBy(int_t ifcModel, int_t ifcObjectInstance, STRUCT_TREE_ITEM *parent, STRUCT_TREE_ITEM	**pChild);
STRUCT_TREE_ITEM** CreateTreeItem_ifcSpaceBoundaryContains(int_t ifcModel, int_t ifcObjectInstance, STRUCT_TREE_ITEM *parent, STRUCT_TREE_ITEM** pChild);
void disPlay();
void glDisPlay(int argc, char *argv[]);
float maxV(float a[], int n);
float minV(float a[], int n);

STRUCT_TREE_ITEM* GetHeaderInfo(int_t ifcModel);
STRUCT_TREE_ITEM* CreateTreeItem__TEXT(STRUCT_TREE_ITEM	*parent, wchar_t *text);
STRUCT_TREE_ITEM* CreateTreeItem_ifcObject(int_t ifcModel, int_t ifcObjectInstance, STRUCT_TREE_ITEM *parent);
void CreateTreeItem_ifcProject(int_t ifcModel);
void CreateTreeItem_ifcSpaceBoundary(int_t ifcModel);
void CreateTreeItem_nonReferencedIfcItems(int_t	ifcModel);
void CreateTreeItem_ifcGroup(int_t ifcModel);
STRUCT__SIUNIT* GetUnits(int_t ifcModel, int_t ifcProjectInstance);
void CreateIfcInstanceProperties(int_t ifcModel, STRUCT__PROPERTY__SET ** propertySets, int_t ifcObjectInstance, STRUCT__SIUNIT * units);

//全局变量定义
wchar_t* ifcSchemaName_IFC2x3 = L"IFC2X3_TC1.exp";
wchar_t* ifcSchemaName_IFC4 = L"IFC4_ADD2.exp";
wchar_t* ifcSchemaName_IFC4x1 = L"IFC4X1.exp";
int_t ifcSpace_TYPE = 0, ifcDistributionElement_TYPE = 0, ifcElectricalElement_TYPE = 0, ifcElementAssembly_TYPE = 0,	ifcElementComponent_TYPE = 0, ifcEquipmentElement_TYPE = 0, ifcFeatureElement_TYPE = 0, ifcFeatureElementSubtraction_TYPE = 0, ifcFurnishingElement_TYPE = 0, ifcReinforcingElement_TYPE = 0, ifcTransportElement_TYPE = 0, ifcVirtualElement_TYPE = 0;
bool firstItemWithGeometryPassed;
STRUCT__IFC__OBJECT *ifcObjectLinkedList = nullptr;
STRUCT_TREE_ITEM *topTreeItem = nullptr, *topModelTreeItem = nullptr, *topSpaceBoundaryTreeItem = nullptr, *topNonReferencedTreeItem = nullptr, *topGroupTreeItem = nullptr;;
int max, min;
std::unordered_map<int_t, STRUCT__IFC__OBJECT *> allIfcObjects;
int_t ifcRelAggregates_TYPE = 0, ifcRelContainedInSpatialStructure_TYPE = 0;
STRUCT__SIUNIT *units = nullptr;


int main(int argc, char* argv[])
{
	wchar_t *ifcFileName;
	ifcFileName = L"myfile1.ifc";

	setStringUnicode(1);
	int_t model = sdaiOpenModelBNUnicode(0, (void*)ifcFileName, 0);

	if (model) {
		wchar_t	*fileSchema = 0;
		GetSPFFHeaderItem(model, 9, 0, sdaiUNICODE, (char**)&fileSchema);
		if (fileSchema == 0 || contains(fileSchema, L"IFC2x3") || contains(fileSchema, L"IFC2X3") || contains(fileSchema, L"IFC2x2") || contains(fileSchema, L"IFC2X2") || contains(fileSchema, L"IFC2x_") || contains(fileSchema, L"IFC2X_") || contains(fileSchema, L"IFC20"))
		{
			sdaiCloseModel(model);
			model = sdaiOpenModelBNUnicode(0, (char*)ifcFileName, (void*)ifcSchemaName_IFC2x3);
		}
		else {
			if (contains(fileSchema, L"IFC4x") || contains(fileSchema, L"IFC4X"))
			{
				sdaiCloseModel(model);
				model = sdaiOpenModelBNUnicode(0, (char*)ifcFileName, (char*)ifcSchemaName_IFC4x1);
			}
			else {
				if (contains(fileSchema, L"IFC4") || contains(fileSchema, L"IFC2x4") || contains(fileSchema, L"IFC2X4"))
				{
					sdaiCloseModel(model);
					model = sdaiOpenModelBNUnicode(0, (char*)ifcFileName, (char*)ifcSchemaName_IFC4);
				}
				else {
					sdaiCloseModel(model);
					return	false;
				}
			}
		}
	}

	ifcSpace_TYPE = sdaiGetEntity(model, (char*)L"IFCSPACE");
	ifcDistributionElement_TYPE = sdaiGetEntity(model, (char*)L"IFCDISTRIBUTIONELEMENT");
	ifcElectricalElement_TYPE = sdaiGetEntity(model, (char*)L"IFCELECTRICALELEMENT");
	ifcElementAssembly_TYPE = sdaiGetEntity(model, (char*)L"IFCELEMENTASSEMBLY");
	ifcElementComponent_TYPE = sdaiGetEntity(model, (char*)L"IFCELEMENTCOMPONENT");
	ifcEquipmentElement_TYPE = sdaiGetEntity(model, (char*)L"IFCEQUIPMENTELEMENT");
	ifcFeatureElement_TYPE = sdaiGetEntity(model, (char*)L"IFCFEATUREELEMENT");
	ifcFeatureElementSubtraction_TYPE = sdaiGetEntity(model, (char*)L"IFCFEATUREELEMENTSUBTRACTION");
	ifcFurnishingElement_TYPE = sdaiGetEntity(model, (char*)L"IFCFURNISHINGELEMENT");
	ifcReinforcingElement_TYPE = sdaiGetEntity(model, (char*)L"IFCREINFORCINGELEMENT");
	ifcTransportElement_TYPE = sdaiGetEntity(model, (char*)L"IFCTRANSPORTELEMENT");
	ifcVirtualElement_TYPE = sdaiGetEntity(model, (char*)L"IFCVIRTUALELEMENT");

	STRUCT__IFC__OBJECT **firstFreeIfcObject = &ifcObjectLinkedList;
	_ASSERT((*firstFreeIfcObject) == nullptr);

	bool hide = false;
	int segmentationParts = 36;
	//下面两行代码得到ifc文件中所有的ifcObj
	firstFreeIfcObject = GetChildrenRecursively(model, sdaiGetEntity(model, (char*)L"IFCPRODUCT"), firstFreeIfcObject, hide, segmentationParts);
	firstFreeIfcObject = queryIfcObjects(model, firstFreeIfcObject, L"IFCRELSPACEBOUNDARY", true, segmentationParts);
	
	//左侧树
	topTreeItem = GetHeaderInfo(model);
	CreateTreeItem_ifcProject(model);
	CreateTreeItem_ifcSpaceBoundary(model);
	CreateTreeItem_nonReferencedIfcItems(model);
	CreateTreeItem_ifcGroup(model);

	STRUCT__IFC__OBJECT *ifcObj = ifcObjectLinkedList;

	//开始循环遍历每一个ifcObject
	while (ifcObj != nullptr)
	{
		wcout << ifcObj->entityName << endl;

		VECTOR3 vecMin = ifcObj->vecMin;
		VECTOR3 vecMax = ifcObj->vecMax;
		
		int_t instance = ifcObj->ifcInstance;
		__int64 noVertices = 0, noIndices = 0;
		
		/*
		计算ifcObject的实例，得到点的个数和索引的个数，
		noVertice表示向量的个数（每一个向量由6个数值组成：点的xyz坐标和点的法向量）
		noIndices表示索引的个数
		*/
		CalculateInstance((__int64)ifcObj->ifcInstance, &noVertices, &noIndices, nullptr);

		int64_t  owlModel = 0, owlInstance = 0;
		
		if (noVertices && noIndices)
		{
			owlGetModel(model, &owlModel);//根据model拿到owlModel
			owlGetInstance(model, ifcObj->ifcInstance, &owlInstance);//根据model和ifcObject的实例拿到owlInstance
		}
		//如果实例有向量和索引，开始寻找这些点
		if (noVertices && noIndices)
		{
			ifcObj->noVertices = (int_t)noVertices;//该object向量个数
			ifcObj->vertices = new float[(int_t)noVertices * 6];//新建向量的存储，每个向量6个数
			int32_t *indices = new int32_t[(int)noIndices];//根据解析出的索引个数建立索引存储

			UpdateInstanceVertexBuffer(owlInstance, ifcObj->vertices);//根据owlInstance，将上步新建的ifcObject实例的存储填入向量的实际内容
			UpdateInstanceIndexBuffer(owlInstance, indices);//根据owlInstance，将上步新建的索引的存储填入索引的实际内容
			//至此，ifcObj->vertices和indices已经保存了当前实例的所有向量和索引
			/*以下内容开始解析这些向量和索引的具体表示什么（三角面face、线line、点point、多边形wireframe）*/

			if (firstItemWithGeometryPassed == false) 
			{
				double	transformationMatrix[12], minVector[3], maxVector[3];
				SetBoundingBoxReference(owlInstance, transformationMatrix, minVector, maxVector);
				if ((-1000000 > transformationMatrix[9] || transformationMatrix[9]  > 1000000) || (-1000000 > transformationMatrix[10] || transformationMatrix[10] > 1000000) || (-1000000 > transformationMatrix[11] || transformationMatrix[11] > 1000000)) 
				{
					SetVertexBufferOffset(model, -transformationMatrix[9], -transformationMatrix[10], -transformationMatrix[11]);
					ClearedInstanceExternalBuffers(owlInstance);
					UpdateInstanceVertexBuffer((__int64)ifcObj->ifcInstance, ifcObj->vertices);
					UpdateInstanceIndexBuffer((__int64)ifcObj->ifcInstance, indices);
				}
			}
			firstItemWithGeometryPassed = true;

			ifcObj->noPrimitivesForWireFrame = 0;
			_ASSERT(ifcObj->noPrimitivesForWireFrame == 0);
			int32_t *indicesForLinesWireFrame = new int32_t[2 * (int)noIndices];

			ifcObj->noVertices = (int_t)noVertices;
			_ASSERT(ifcObj->indicesForFaces == 0);
			int32_t *indicesForFaces = new int32_t[(int)noIndices], *indicesForLines = new int32_t[(int)noIndices], *indicesForPoints = new int32_t[(int)noIndices];

			int_t faceCnt = getConceptualFaceCnt(ifcObj->ifcInstance);//此处的faceCnt表示一个ifcObject的面的个数，此处的face包括点、线、面和多边形
			int_t *maxIndex = new int_t[faceCnt], *primitivesForFaces = new int_t[faceCnt];

			//然后开始解析每个面的具体的表示方式
			for (int_t j = 0; j < faceCnt; j++)
			{
				int_t startIndexTriangles = 0, noIndicesTrangles = 0,
					startIndexLines = 0, noIndicesLines = 0,
					startIndexPoints = 0, noIndicesPoints = 0,
					startIndexFacesPolygons = 0, noIndicesFacesPolygons = 0;

				//得到当前面中组成三角面、线、点和多边形的点在ifcObj->vertice点数组的起始索引和长度
				getConceptualFaceEx(ifcObj->ifcInstance, j, 
					&startIndexTriangles, &noIndicesTrangles,
					&startIndexLines, &noIndicesLines,
					&startIndexPoints, &noIndicesPoints,
					&startIndexFacesPolygons, &noIndicesFacesPolygons,
					0, 0 
				);
				if (j) 
				{
					maxIndex[j] = maxIndex[j - 1];
				}
				else 
				{
					maxIndex[j] = 0;
				}

				if (noIndicesTrangles  &&  maxIndex[j] < startIndexTriangles + noIndicesTrangles) 
				{
					maxIndex[j] = startIndexTriangles + noIndicesTrangles; 
				}
				if (noIndicesLines  &&  maxIndex[j] < startIndexLines + noIndicesLines) 
				{
					maxIndex[j] = startIndexLines + noIndicesLines;
				}
				if (noIndicesPoints  &&  maxIndex[j] < startIndexPoints + noIndicesPoints) 
				{ 
					maxIndex[j] = startIndexPoints + noIndicesPoints;
				}
				if (noIndicesFacesPolygons  &&  maxIndex[j] < startIndexFacesPolygons + noIndicesFacesPolygons) 
				{ 
					maxIndex[j] = startIndexFacesPolygons + noIndicesFacesPolygons; 
				}
				//开始将indices里三角面的索引向只存储三角面索引的indicesForFaces里存储，然后将ifcObj的三角面个数+=索引的1/3
				int_t	i = 0;
				while (i < noIndicesTrangles) {
					indicesForFaces[ifcObj->noPrimitivesForFaces * 3 + i] = indices[startIndexTriangles + i];
					i++;
				}
				ifcObj->noPrimitivesForFaces += noIndicesTrangles / 3;
				primitivesForFaces[j] = noIndicesTrangles / 3;


				//开始将indices里线的索引向只存储线索引的indicesForLines里存储，然后将ifcObj的线的个数+=索引的1/2
				i = 0;
				while (i < noIndicesLines) {
					indicesForLines[ifcObj->noPrimitivesForLines * 2 + i] = indices[startIndexLines + i];
					i++;
				}
				ifcObj->noPrimitivesForLines += noIndicesLines / 2;


				i = 0;
				while (i < noIndicesPoints) {
					indicesForPoints[ifcObj->noPrimitivesForPoints * 1 + i] = indices[startIndexPoints + i];
					i++;
				}
				ifcObj->noPrimitivesForPoints += noIndicesPoints / 1;

				i = 0;
				int32_t	lastItem = -1;
				while (i < noIndicesFacesPolygons) {
					if (lastItem >= 0 && indices[startIndexFacesPolygons + i] >= 0) {
						indicesForLinesWireFrame[2 * ifcObj->noPrimitivesForWireFrame + 0] = lastItem;
						indicesForLinesWireFrame[2 * ifcObj->noPrimitivesForWireFrame + 1] = indices[startIndexFacesPolygons + i];
						ifcObj->noPrimitivesForWireFrame++;
					}
					lastItem = indices[startIndexFacesPolygons + i];
					i++;
				}//while
			}//for

			//for循环结束，当前ifcObject的所有面解析完毕，将实际索引值存入ifcObj对象
			ifcObj->indicesForPoints = new int32_t[3 * ifcObj->noPrimitivesForPoints];
			ifcObj->indicesForLines = new int32_t[3 * ifcObj->noPrimitivesForLines];
			ifcObj->indicesForFaces = new int32_t[3 * ifcObj->noPrimitivesForFaces];
			ifcObj->indicesForLinesWireFrame = new int32_t[2 * ifcObj->noPrimitivesForWireFrame];

			memcpy(ifcObj->indicesForPoints, indicesForPoints, 1 * ifcObj->noPrimitivesForPoints * sizeof(int32_t));
			memcpy(ifcObj->indicesForLines, indicesForLines, 2 * ifcObj->noPrimitivesForLines * sizeof(int32_t));
			memcpy(ifcObj->indicesForFaces, indicesForFaces, 3 * ifcObj->noPrimitivesForFaces * sizeof(int32_t));
			memcpy(ifcObj->indicesForLinesWireFrame, indicesForLinesWireFrame, 2 * ifcObj->noPrimitivesForWireFrame * sizeof(int32_t));

			delete[] indicesForLinesWireFrame;
			delete[] indicesForFaces;
			delete[] indicesForLines;
			delete[] indicesForPoints;
			delete[] indices;
		}

		/*int_t fNum = ifcObj->noPrimitivesForFaces,
			pNum = ifcObj->noPrimitivesForPoints,
			lNum = ifcObj->noPrimitivesForLines,
			wfNum = ifcObj->noPrimitivesForWireFrame;
		cout << "faceNum:" << fNum << ", pointsNum:" << pNum << ", linesNum:" << lNum << ", wireframeNum:" << wfNum << endl;

		/*for (int i = 0; i < ifcObj->noPrimitivesForFaces * 3; i++)
		{
			cout << ifcObj->vertices[ifcObj->indicesForFaces[i] * 6] << "," << ifcObj->vertices[ifcObj->indicesForFaces[i] * 6 + 1] << "," << ifcObj->vertices[ifcObj->indicesForFaces[i] * 6 + 2] << endl;
		}*/
		/*cout << ifcObj->ifcEntity << endl;
		cout << "当前obj的几何顶点信息:" << endl;
		cout << "总长度:" << ifcObj->noVertices << endl;
		cout << ifcObj->noPrimitivesForPoints << "," << ifcObj->indexOffsetForPoints << "," << ifcObj->noPrimitivesForLines << "," << ifcObj->indexOffsetForLines << "," << ifcObj->noPrimitivesForFaces << "," << ifcObj->indexOffsetForFaces << "," << ifcObj->noPrimitivesForWireFrame << "," << ifcObj->indexOffsetForWireFrame << endl;
		*/

		float currentMax = maxV(ifcObj->vertices, noVertices * 6);
		float currentMin = minV(ifcObj->vertices, noVertices * 6);
		if (currentMax > max)
		{
			max = currentMax;
		}
		if (currentMin < min)
		{
			min = currentMin;
		}
		STRUCT__PROPERTY__SET *propertySet = nullptr;
		//createtooltip
		CreateIfcInstanceProperties(model, &propertySet, ifcObj->ifcInstance, units);
		if (propertySet && propertySet->properties)
		{
			wcout << ifcObj->entityName << ", " << propertySet->properties->name << ", " << propertySet->properties->nominalValue << endl;
			wcout << propertySet->nameBuffer << endl;
		}

		ifcObj = ifcObj->next;
	}
	
	//根据vertices和索引来画图
	glDisPlay(argc, argv);
	
	//topTreeItem表示左侧树形结构的节点，next指针指向下一级兄弟节点，child指针指子节点。一般由4个节点组成（header info，ifcproject'buildingtemplate', not referenced, groups）
	STRUCT_TREE_ITEM *stt = new STRUCT_TREE_ITEM;
	stt = topTreeItem;
	int i = 0;
	while (stt)
	{
		i++;
		if (stt->nameBuffer != NULL && stt->nameBuffer != L"")
		{
			wcout << stt->nameBuffer << endl;
		}
		
		stt = stt->next;
	}
	cout << i << endl;
	system("pause");
	return 0;
}


//函数实现
bool contains(wchar_t *txtI, wchar_t *txtII)
{
	int_t	i = 0;
	while (txtI[i] && txtII[i]) {
		if (txtI[i] != txtII[i]) {
			return	false;
		}
		i++;
	}
	if (txtII[i]) {
		return	false;
	}
	else {
		return	true;
	}
}
STRUCT__IFC__OBJECT	* CreateIfcObject(int_t ifcEntity, int_t ifcInstance, wchar_t * entityName, bool hide, int_t segmentationParts)
{
	STRUCT__IFC__OBJECT	* ifcObject = new STRUCT__IFC__OBJECT;

	if (hide) {
		ifcObject->selectState = TI_UNCHECKED;
	}
	else {
		ifcObject->selectState = TI_CHECKED;
	}

	ifcObject->noVertices = 0;
	ifcObject->vertices = 0;

	ifcObject->next = nullptr;

	ifcObject->entityName = entityName;
	ifcObject->hide = hide;
	ifcObject->segmentationParts = segmentationParts;
	ifcObject->treeItemModel = nullptr;
	ifcObject->treeItemSpaceBoundary = nullptr;
	ifcObject->treeItemNonReferenced = nullptr;

	ifcObject->materials = 0;

	ifcObject->vecMin.x = 0;
	ifcObject->vecMin.y = 0;
	ifcObject->vecMin.z = 0;
	ifcObject->vecMax.x = 0;
	ifcObject->vecMax.y = 0;
	ifcObject->vecMax.z = 0;

	ifcObject->ifcInstance = ifcInstance;
	ifcObject->ifcEntity = ifcEntity;
	//	if	(hide) {
	//		ifcObject->ifcItemCheckedAtStartup = false;
	//	} else {
	//		ifcObject->ifcItemCheckedAtStartup = true;
	//	}
	//	ifcObject->treeItemGeometry = 0;
	//	ifcObject->treeItemProperties = 0;

	ifcObject->noVertices = 0;
	ifcObject->vertices = 0;

	ifcObject->vertexOffset = 0;

	ifcObject->noPrimitivesForPoints = 0;
	ifcObject->indicesForPoints = 0;
	ifcObject->indexOffsetForPoints = 0;

	ifcObject->noPrimitivesForLines = 0;
	ifcObject->indicesForLines = 0;
	ifcObject->indexOffsetForLines = 0;

	ifcObject->noPrimitivesForFaces = 0;
	ifcObject->indicesForFaces = 0;
	ifcObject->indexOffsetForFaces = 0;

	ifcObject->noPrimitivesForWireFrame = 0;
	ifcObject->indicesForLinesWireFrame = 0;
	ifcObject->indexOffsetForWireFrame = 0;

	return	ifcObject;
}
STRUCT__IFC__OBJECT** queryIfcObjects(int_t ifcModel, STRUCT__IFC__OBJECT	**firstFreeIfcObject, wchar_t *entityName, bool hide, int_t	segmentationParts)
{
	int_t i, *ifcObjectInstances, noIfcObjectInstances;

	ifcObjectInstances = sdaiGetEntityExtentBN(ifcModel, (char*)entityName);

	noIfcObjectInstances = sdaiGetMemberCount(ifcObjectInstances);
	if (noIfcObjectInstances) {
		int_t	ifcEntity = sdaiGetEntity(ifcModel, (char*)entityName);
		for (i = 0; i < noIfcObjectInstances; ++i) {
			int_t	ifcObjectInstance = 0;
			engiGetAggrElement(ifcObjectInstances, i, sdaiINSTANCE, &ifcObjectInstance);

			STRUCT__IFC__OBJECT	* ifcObject = CreateIfcObject(ifcEntity, ifcObjectInstance, entityName, hide, segmentationParts);
			(*firstFreeIfcObject) = ifcObject;
			firstFreeIfcObject = &ifcObject->next;

			allIfcObjects[ifcObjectInstance] = ifcObject;
		}
	}

	return	firstFreeIfcObject;
}
STRUCT__IFC__OBJECT** GetChildrenRecursively(int_t ifcModel, int_t ifcParentEntity, STRUCT__IFC__OBJECT **firstFreeIfcObject, bool hide, int_t segmentationParts)
{


	int_t	*ifcEntityExtend = sdaiGetEntityExtent(ifcModel, ifcParentEntity), cnt = sdaiGetMemberCount(ifcEntityExtend);

	if ((ifcParentEntity == ifcSpace_TYPE) || (ifcParentEntity == ifcFeatureElementSubtraction_TYPE))
	{
		hide = true;
	}

	if ((ifcParentEntity == ifcDistributionElement_TYPE) || (ifcParentEntity == ifcElectricalElement_TYPE) || (ifcParentEntity == ifcElementAssembly_TYPE) || (ifcParentEntity == ifcElementComponent_TYPE) || (ifcParentEntity == ifcEquipmentElement_TYPE) || (ifcParentEntity == ifcFeatureElement_TYPE) || (ifcParentEntity == ifcFurnishingElement_TYPE) || (ifcParentEntity == ifcTransportElement_TYPE) || (ifcParentEntity == ifcVirtualElement_TYPE))
	{
		segmentationParts = 12;
	}

	if (ifcParentEntity == ifcReinforcingElement_TYPE) {
		segmentationParts = 6;// 12;
	}

	if (cnt) {
		wchar_t	* ifcParentEntityName = nullptr;
		engiGetEntityName(ifcParentEntity, sdaiUNICODE, (char**)&ifcParentEntityName);
		firstFreeIfcObject = queryIfcObjects(ifcModel, firstFreeIfcObject, ifcParentEntityName, hide, segmentationParts);
	}

	cnt = engiGetEntityCount(ifcModel);
	for (int_t i = 0; i < cnt; i++)
	{
		int_t ifcEntity = engiGetEntityElement(ifcModel, i);
		if (engiGetEntityParent(ifcEntity) == ifcParentEntity) {
			firstFreeIfcObject = GetChildrenRecursively(ifcModel, ifcEntity, firstFreeIfcObject, hide, segmentationParts);
		}
	}
	return	firstFreeIfcObject;
}
void disPlay()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glTranslatef(3.0, 0.0, 0.0);
	glRotated(45, 0, -1.0, 1.0);
	glScaled(1.2, 1.2, 1.2);
	STRUCT__IFC__OBJECT *ifcObj = ifcObjectLinkedList;
	int distance = max - min;
	FILE *fos = fopen("F:/data/three.txt", "a");

	while (ifcObj != nullptr)
	{
		for (int i = 0; i < ifcObj->noPrimitivesForFaces * 3; i = i + 3)
		{
			float x[3], y[3], z[3];
			x[0] = ifcObj->vertices[ifcObj->indicesForFaces[i] * 6];
			x[1] = ifcObj->vertices[ifcObj->indicesForFaces[i + 1] * 6];
			x[2] = ifcObj->vertices[ifcObj->indicesForFaces[i + 2] * 6];
			y[0] = ifcObj->vertices[ifcObj->indicesForFaces[i] * 6 + 1];
			y[1] = ifcObj->vertices[ifcObj->indicesForFaces[i + 1] * 6 + 1];
			y[2] = ifcObj->vertices[ifcObj->indicesForFaces[i + 2] * 6 + 1];
			z[0] = ifcObj->vertices[ifcObj->indicesForFaces[i] * 6 + 2];
			z[1] = ifcObj->vertices[ifcObj->indicesForFaces[i + 1] * 6 + 2];
			z[2] = ifcObj->vertices[ifcObj->indicesForFaces[i + 2] * 6 + 2];
			fprintf(fos, "%lf, %lf, %lf\n", (double)x[0] / (double)distance, (double)y[0] / (double)distance, (double)z[0] / (double)distance);
			fprintf(fos, "%lf, %lf, %lf\n", (double)x[1] / (double)distance, (double)y[1] / (double)distance, (double)z[1] / (double)distance);
			fprintf(fos, "%lf, %lf, %lf\n", (double)x[2] / (double)distance, (double)y[2] / (double)distance, (double)z[2] / (double)distance);
			glBegin(GL_TRIANGLES);
			glVertex3d((double)x[0] / (double)distance, (double)y[0] / (double)distance, (double)z[0] / (double)distance);
			glVertex3d((double)x[1] / (double)distance, (double)y[1] / (double)distance, (double)z[1] / (double)distance);
			glVertex3d((double)x[2] / (double)distance, (double)y[2] / (double)distance, (double)z[2] / (double)distance);
			glEnd();
			glFlush();
		}
		ifcObj = ifcObj->next;
	}
	fclose(fos);
}
void glDisPlay(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(400, 400);
	glutCreateWindow("第一个OpenGL程序");
	glutDisplayFunc(&disPlay);
	glutMainLoop();
}
float maxV(float a[], int n)
{
	float max = 0;
	if (n > 0)
	{
		max = a[0];
		for (int i = 1; i < n; i++)
		{
			if (max < a[i])
			{
				max = a[i];
			}
		}
	}
	return max;
}
float minV(float a[], int n)
{
	float min = 0;
	if (n > 0)
	{
		min = a[0];
		for (int i = 1; i < n; i++)
		{
			if (min > a[i])
			{
				min = a[i];
			}
		}
	}
	return min;
}

//得到header info
void InitTreeItem(STRUCT_TREE_ITEM *treeItem, STRUCT_TREE_ITEM *parent)
{
	_ASSERT(treeItem->type);
	treeItem->nameBuffer = nullptr;
	treeItem->hTreeItem = 0;
	treeItem->parent = parent;
	treeItem->child = nullptr;
	treeItem->next = nullptr;
}
STRUCT_TREE_ITEM* CreateTreeItem__TEXT(STRUCT_TREE_ITEM	*parent, wchar_t *text)
{
	STRUCT_TREE_ITEM_TEXT	*treeItem = new STRUCT_TREE_ITEM_TEXT;
	treeItem->type = TREE_ITEM_TEXT;
	InitTreeItem(treeItem, parent);
	treeItem->nameBuffer = text;
	treeItem->allocated = false;
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__ALLOCATEDTEXT(STRUCT_TREE_ITEM *parent, wchar_t *text)
{
	STRUCT_TREE_ITEM_TEXT	* treeItem = new STRUCT_TREE_ITEM_TEXT;
	treeItem->type = TREE_ITEM_TEXT;
	InitTreeItem(treeItem, parent);
	treeItem->nameBuffer = text;
	treeItem->allocated = true;
	return	treeItem;
}
wchar_t* CombineText(wchar_t *textI, wchar_t *textII)
{
	size_t	lenTextI = 0, lenTextII = 0;
	if (textI) {
		lenTextI = wcslen(textI);
	}
	if (textII) {
		lenTextII = wcslen(textII);
	}
	wchar_t* text = new wchar_t[lenTextI + 4 + lenTextII + 2];
	if (textI) {
		memcpy(&text[0], textI, lenTextI * sizeof(wchar_t));
	}
	text[lenTextI + 0] = ' ';
	text[lenTextI + 1] = '=';
	text[lenTextI + 2] = ' ';
	text[lenTextI + 3] = '\'';
	if (textII) {
		memcpy(&text[lenTextI + 4], textII, lenTextII * sizeof(wchar_t));
	}
	text[lenTextI + 4 + lenTextII + 0] = '\'';
	text[lenTextI + 4 + lenTextII + 1] = 0;

	return	text;
}
STRUCT_TREE_ITEM* GetHeaderDescription(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM *headerDescription = CreateTreeItem__TEXT(parent, L"Set of Descriptions"),
		**ppHeader = &headerDescription->child;

	wchar_t	*text = nullptr;
	int_t	i = 0;

	if (!GetSPFFHeaderItem(ifcModel, 0, i, sdaiUNICODE, (char **)&text)) {
		while (!GetSPFFHeaderItem(ifcModel, 0, i++, sdaiUNICODE, (char **)&text)) {
			(*ppHeader) = CreateTreeItem__ALLOCATEDTEXT(headerDescription, CombineText(L"Description", text));
			ppHeader = &(*ppHeader)->next;
			text = nullptr;
		}
	}

	return	headerDescription;
}
STRUCT_TREE_ITEM* GetImplementationLevel(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM	* headerImplementationLevel = nullptr;
	wchar_t	* text = nullptr;
	GetSPFFHeaderItem(ifcModel, 1, 0, sdaiUNICODE, (char **)&text);
	headerImplementationLevel = CreateTreeItem__ALLOCATEDTEXT(parent, CombineText(L"ImplementationLevel", text));

	return	headerImplementationLevel;
}
STRUCT_TREE_ITEM* GetName(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM	* headerName = 0;

	wchar_t	* text = nullptr;

	GetSPFFHeaderItem(ifcModel, 2, 0, sdaiUNICODE, (char **)&text);
	headerName = CreateTreeItem__ALLOCATEDTEXT(parent, CombineText(L"Name", text));

	return	headerName;
}
STRUCT_TREE_ITEM* GetTimeStamp(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM* headerTimeStamp = nullptr;

	wchar_t* text = nullptr;

	GetSPFFHeaderItem(ifcModel, 3, 0, sdaiUNICODE, (char **)&text);
	headerTimeStamp = CreateTreeItem__ALLOCATEDTEXT(parent, CombineText(L"TimeStamp", text));

	return	headerTimeStamp;
}
STRUCT_TREE_ITEM* GetAuthor(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM* headerAuthor = CreateTreeItem__TEXT(parent, L"Set of Authors"),
		**ppHeader = &headerAuthor->child;

	wchar_t* text = nullptr;
	int_t	i = 0;

	if (!GetSPFFHeaderItem(ifcModel, 4, i, sdaiUNICODE, (char **)&text)) {
		while (!GetSPFFHeaderItem(ifcModel, 4, i++, sdaiUNICODE, (char **)&text)) {
			(*ppHeader) = CreateTreeItem__ALLOCATEDTEXT(headerAuthor, CombineText(L"Author", text));
			ppHeader = &(*ppHeader)->next;
			text = 0;
		}
	}

	return	headerAuthor;
}
STRUCT_TREE_ITEM* GetOrganization(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM* headerOrganization = CreateTreeItem__TEXT(parent, L"Set of Organizations"),
		**ppHeader = &headerOrganization->child;

	wchar_t	* text = nullptr;
	int_t	i = 0;

	if (!GetSPFFHeaderItem(ifcModel, 5, i, sdaiUNICODE, (char **)&text)) {
		while (!GetSPFFHeaderItem(ifcModel, 5, i++, sdaiUNICODE, (char **)&text)) {
			(*ppHeader) = CreateTreeItem__ALLOCATEDTEXT(headerOrganization, CombineText(L"Organization", text));
			ppHeader = &(*ppHeader)->next;
			text = 0;
		}
	}

	return	headerOrganization;
}
STRUCT_TREE_ITEM* GetPreprocessorVersion(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM* headerPreprocessorVersion = nullptr;
	wchar_t* text = nullptr;
	GetSPFFHeaderItem(ifcModel, 6, 0, sdaiUNICODE, (char **)&text);
	headerPreprocessorVersion = CreateTreeItem__ALLOCATEDTEXT(parent, CombineText(L"PreprocessorVersion", text));
	return	headerPreprocessorVersion;
}
STRUCT_TREE_ITEM* GetOriginatingSystem(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM* headerOriginatingSystem = nullptr;

	wchar_t* text = nullptr;

	GetSPFFHeaderItem(ifcModel, 7, 0, sdaiUNICODE, (char **)&text);
	headerOriginatingSystem = CreateTreeItem__ALLOCATEDTEXT(parent, CombineText(L"OriginatingSystem", text));

	return	headerOriginatingSystem;
}
STRUCT_TREE_ITEM* GetAuthorization(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM* headerAuthorization = nullptr;

	wchar_t* text = nullptr;

	GetSPFFHeaderItem(ifcModel, 8, 0, sdaiUNICODE, (char **)&text);
	headerAuthorization = CreateTreeItem__ALLOCATEDTEXT(parent, CombineText(L"Authorization", text));

	return	headerAuthorization;
}
STRUCT_TREE_ITEM* GetFileSchema(int_t ifcModel, STRUCT_TREE_ITEM * parent)
{
	STRUCT_TREE_ITEM* headerFileSchema = CreateTreeItem__TEXT(parent, L"Set of FileSchemas"),
		**ppHeader = &headerFileSchema->child;

	wchar_t	* text = nullptr;
	int_t	i = 0;

	if (!GetSPFFHeaderItem(ifcModel, 9, i, sdaiUNICODE, (char **)&text)) {
		while (!GetSPFFHeaderItem(ifcModel, 9, i++, sdaiUNICODE, (char **)&text)) {
			(*ppHeader) = CreateTreeItem__ALLOCATEDTEXT(headerFileSchema, CombineText(L"FileSchema", text));
			ppHeader = &(*ppHeader)->next;
			text = 0;
		}
	}

	return	headerFileSchema;
}
STRUCT_TREE_ITEM* GetHeaderInfo(int_t ifcModel)
{
	STRUCT_TREE_ITEM *headerFileSchema = CreateTreeItem__TEXT(nullptr, L"Header Info"),
		             **ppHeader = &headerFileSchema->child;

	(*ppHeader) = GetHeaderDescription(ifcModel, headerFileSchema);
	ppHeader = &(*ppHeader)->next;
	(*ppHeader) = GetImplementationLevel(ifcModel, headerFileSchema);
	ppHeader = &(*ppHeader)->next;
	(*ppHeader) = GetName(ifcModel, headerFileSchema);
	ppHeader = &(*ppHeader)->next;
	(*ppHeader) = GetTimeStamp(ifcModel, headerFileSchema);
	ppHeader = &(*ppHeader)->next;
	(*ppHeader) = GetAuthor(ifcModel, headerFileSchema);
	ppHeader = &(*ppHeader)->next;
	(*ppHeader) = GetOrganization(ifcModel, headerFileSchema);
	ppHeader = &(*ppHeader)->next;
	(*ppHeader) = GetPreprocessorVersion(ifcModel, headerFileSchema);
	ppHeader = &(*ppHeader)->next;
	(*ppHeader) = GetOriginatingSystem(ifcModel, headerFileSchema);
	ppHeader = &(*ppHeader)->next;
	(*ppHeader) = GetAuthorization(ifcModel, headerFileSchema);
	ppHeader = &(*ppHeader)->next;
	(*ppHeader) = GetFileSchema(ifcModel, headerFileSchema);
	return	headerFileSchema;
}

//CreateTreeItem_ifcProject
void InitTreeItemSelectable(STRUCT_TREE_ITEM_SELECTABLE	*treeItem, STRUCT_TREE_ITEM *parent)
{
	InitTreeItem(treeItem, parent);
	treeItem->selectState = TI_CHECKED;
}
STRUCT_TREE_ITEM* CreateTreeItem__IFCINSTANCE_model(STRUCT_TREE_ITEM* parent, int_t ifcModel, int_t ifcInstance)
{
	STRUCT_TREE_ITEM_IFCINSTANCE	* treeItem = new STRUCT_TREE_ITEM_IFCINSTANCE;
	treeItem->type = TREE_ITEM_IFCINSTANCE;
	InitTreeItemSelectable(treeItem, parent);
	treeItem->ifcModel = ifcModel;
	treeItem->ifcInstance = ifcInstance;

	treeItem->ifcObject = allIfcObjects[ifcInstance];
	if (treeItem->ifcObject) {
		treeItem->ifcObject->treeItemModel = treeItem;
	}
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__GEOMETRY(STRUCT_TREE_ITEM	*parent)
{
	STRUCT_TREE_ITEM_GEOMETRY	*treeItem = new STRUCT_TREE_ITEM_GEOMETRY;
	treeItem->type = TREE_ITEM_GEOMETRY;
	InitTreeItemSelectable(treeItem, parent);
	treeItem->nameBuffer = L"geometry";
	_ASSERT(treeItem->parent  &&  treeItem->parent->type == TREE_ITEM_IFCINSTANCE);
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__PROPERTIES(STRUCT_TREE_ITEM *parent)
{
	STRUCT_TREE_ITEM_PROPERTIES	* treeItem = new STRUCT_TREE_ITEM_PROPERTIES;
	treeItem->type = TREE_ITEM_PROPERTIES;
	InitTreeItem(treeItem, parent);
	treeItem->nameBuffer = L"properties";
	_ASSERT(treeItem->parent  &&  treeItem->parent->type == TREE_ITEM_IFCINSTANCE);
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__DECOMPOSEDBY(STRUCT_TREE_ITEM	* parent)
{
	STRUCT_TREE_ITEM_DECOMPOSEDBY	* treeItem = new STRUCT_TREE_ITEM_DECOMPOSEDBY;
	treeItem->type = TREE_ITEM_DECOMPOSEDBY;
	InitTreeItemSelectable(treeItem, parent);
	treeItem->nameBuffer = L"decomposition";
	_ASSERT(treeItem->parent  &&  treeItem->parent->type == TREE_ITEM_IFCINSTANCE);
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem_ifcObjectDecomposedBy(int_t ifcModel, int_t ifcObjectInstance, STRUCT_TREE_ITEM	*parent)
{
	STRUCT_TREE_ITEM	* decomposedByTreeItem = nullptr, ** ppChild = nullptr;
	int_t	* ifcRelDecomposesInstances = nullptr, ifcRelDecomposesInstancesCnt;
	sdaiGetAttrBN(ifcObjectInstance, (char*)L"IsDecomposedBy", sdaiAGGR, &ifcRelDecomposesInstances);
	if (ifcRelDecomposesInstances) {
		ifcRelDecomposesInstancesCnt = sdaiGetMemberCount(ifcRelDecomposesInstances);
		for (int_t j = 0; j < ifcRelDecomposesInstancesCnt; ++j) {
			int_t ifcRelDecomposesInstance = 0;
			engiGetAggrElement(ifcRelDecomposesInstances, j, sdaiINSTANCE, &ifcRelDecomposesInstance);
			if (sdaiGetInstanceType(ifcRelDecomposesInstance) == ifcRelAggregates_TYPE) {
				int_t	* ifcObjectInstances = nullptr;
				sdaiGetAttrBN(ifcRelDecomposesInstance, (char*)L"RelatedObjects", sdaiAGGR, &ifcObjectInstances);

				int_t	ifcObjectInstancesCnt = sdaiGetMemberCount(ifcObjectInstances);
				if (ifcObjectInstancesCnt) {
					if (decomposedByTreeItem == nullptr) {
						decomposedByTreeItem = CreateTreeItem__DECOMPOSEDBY(parent);
						ppChild = &decomposedByTreeItem->child;
					}
					for (int_t k = 0; k < ifcObjectInstancesCnt; ++k) {
						ifcObjectInstance = 0;
						engiGetAggrElement(ifcObjectInstances, k, sdaiINSTANCE, &ifcObjectInstance);

						(*ppChild) = CreateTreeItem_ifcObject(ifcModel, ifcObjectInstance, decomposedByTreeItem);
						ppChild = &(*ppChild)->next;
					}
				}
			}
			else {
				_ASSERT(false);
			}
		}
	}

	return	decomposedByTreeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__CONTAINS(STRUCT_TREE_ITEM	* parent)
{
	STRUCT_TREE_ITEM_CONTAINS* treeItem = new STRUCT_TREE_ITEM_CONTAINS;
	treeItem->type = TREE_ITEM_CONTAINS;
	InitTreeItemSelectable(treeItem, parent);
	treeItem->nameBuffer = L"contains";
	_ASSERT(treeItem->parent  &&  treeItem->parent->type == TREE_ITEM_IFCINSTANCE);
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem_ifcObjectContains(int_t ifcModel, int_t ifcObjectInstance, STRUCT_TREE_ITEM* parent)
{
	STRUCT_TREE_ITEM	* containsTreeItem = nullptr, ** ppChild = nullptr;
	int_t	* ifcRelContainedInSpatialStructureInstances = nullptr;
	sdaiGetAttrBN(ifcObjectInstance, (char*)L"ContainsElements", sdaiAGGR, &ifcRelContainedInSpatialStructureInstances);
	int_t	ifcRelContainedInSpatialStructureInstancesCnt = sdaiGetMemberCount(ifcRelContainedInSpatialStructureInstances);

	for (int_t i = 0; i < ifcRelContainedInSpatialStructureInstancesCnt; ++i) {
		int_t	ifcRelContainedInSpatialStructureInstance = 0;
		engiGetAggrElement(ifcRelContainedInSpatialStructureInstances, i, sdaiINSTANCE, &ifcRelContainedInSpatialStructureInstance);
		if (sdaiGetInstanceType(ifcRelContainedInSpatialStructureInstance) == ifcRelContainedInSpatialStructure_TYPE) {
			int_t	* ifcObjectInstances = nullptr;
			sdaiGetAttrBN(ifcRelContainedInSpatialStructureInstance, (char*)L"RelatedElements", sdaiAGGR, &ifcObjectInstances);

			int_t	ifcObjectInstancesCnt = sdaiGetMemberCount(ifcObjectInstances);
			if (ifcObjectInstancesCnt) {
				if (containsTreeItem == nullptr) {
					containsTreeItem = CreateTreeItem__CONTAINS(parent);
					ppChild = &containsTreeItem->child;
				}

				for (int_t k = 0; k < ifcObjectInstancesCnt; ++k) {
					ifcObjectInstance = 0;
					engiGetAggrElement(ifcObjectInstances, k, sdaiINSTANCE, &ifcObjectInstance);

					(*ppChild) = CreateTreeItem_ifcObject(ifcModel, ifcObjectInstance, containsTreeItem);
					ppChild = &(*ppChild)->next;
				}
			}
		}
		else {
			_ASSERT(false);
		}
	}

	return	containsTreeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem_ifcObject(int_t ifcModel, int_t ifcObjectInstance, STRUCT_TREE_ITEM *parent)
{
	STRUCT_TREE_ITEM	* treeItem = CreateTreeItem__IFCINSTANCE_model(parent, ifcModel, ifcObjectInstance);
	treeItem->child = CreateTreeItem__GEOMETRY(treeItem);
	treeItem->child->next = CreateTreeItem__PROPERTIES(treeItem);
	treeItem->child->next->next = CreateTreeItem_ifcObjectDecomposedBy(ifcModel, ifcObjectInstance, treeItem);
	if (treeItem->child->next->next) {
		treeItem->child->next->next->next = CreateTreeItem_ifcObjectContains(ifcModel, ifcObjectInstance, treeItem);
	}
	else {
		treeItem->child->next->next = CreateTreeItem_ifcObjectContains(ifcModel, ifcObjectInstance, treeItem);
	}
	return	treeItem;
}
void CreateTreeItem_ifcProject(int_t ifcModel)
{
	ifcRelAggregates_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCRELAGGREGATES");
	ifcRelContainedInSpatialStructure_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCRELCONTAINEDINSPATIALSTRUCTURE");

	int_t *ifcProjectInstances = sdaiGetEntityExtentBN(ifcModel, (char*)L"IFCPROJECT"),
		  noIfcProjectInstances = sdaiGetMemberCount(ifcProjectInstances);

	cout << "noIfcProjectInstances:" << noIfcProjectInstances <<endl;

	for (int_t i = 0; i < noIfcProjectInstances; ++i) 
	{
		int_t ifcProjectInstance = 0;
		engiGetAggrElement(ifcProjectInstances, i, sdaiINSTANCE, &ifcProjectInstance);
		units = GetUnits(ifcModel, ifcProjectInstance);
		STRUCT_TREE_ITEM *treeItemIfcProject = CreateTreeItem_ifcObject(ifcModel, ifcProjectInstance, nullptr);
		_ASSERT(treeItemIfcProject->next == nullptr);
		topModelTreeItem = treeItemIfcProject;
		treeItemIfcProject->next = topTreeItem;
		topTreeItem = treeItemIfcProject;
	}
}

//CreateTreeItem_ifcSpaceBoundary
STRUCT_TREE_ITEM* CreateTreeItem__SPACEBOUNDARIES(STRUCT_TREE_ITEM *parent)
{
	STRUCT_TREE_ITEM_SPACEBOUNDARIES *treeItem = new STRUCT_TREE_ITEM_SPACEBOUNDARIES;
	treeItem->type = TREE_ITEM_SPACEBOUNDARIES;
	InitTreeItemSelectable(treeItem, parent);
	treeItem->nameBuffer = L"Space Boundaries";
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__IFCINSTANCE_spaceBoundary(STRUCT_TREE_ITEM *parent, int_t ifcModel, int_t	ifcInstance)
{
	STRUCT_TREE_ITEM_IFCINSTANCE *treeItem = new STRUCT_TREE_ITEM_IFCINSTANCE;
	treeItem->type = TREE_ITEM_IFCINSTANCE;
	InitTreeItemSelectable(treeItem, parent);

	//	Init Tree Item IfcInstance
	treeItem->ifcModel = ifcModel;
	treeItem->ifcInstance = ifcInstance;
	treeItem->ifcObject = allIfcObjects[ifcInstance];
	if (treeItem->ifcObject) {
		treeItem->ifcObject->treeItemSpaceBoundary = treeItem;
	}
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem_ifcSpaceBoundary_SPACE(int_t ifcModel, int_t ifcSpaceInstance, STRUCT_TREE_ITEM* parent)
{
	int_t *ifcRelSpaceBoundaryInstanceSet = nullptr;
	sdaiGetAttrBN(ifcSpaceInstance, (char*)L"BoundedBy", sdaiAGGR, &ifcRelSpaceBoundaryInstanceSet);
	int_t ifcObjectDefinitionInstanceSetCnt = sdaiGetMemberCount(ifcRelSpaceBoundaryInstanceSet);
	if (ifcObjectDefinitionInstanceSetCnt) {
		STRUCT_TREE_ITEM	*treeItem = CreateTreeItem__SPACEBOUNDARIES(parent),
			**pChild = &treeItem->child;

		for (int_t i = 0; i < ifcObjectDefinitionInstanceSetCnt; i++) {
			int_t	ifcRelSpaceBoundaryInstance = 0;
			engiGetAggrElement(ifcRelSpaceBoundaryInstanceSet, i, sdaiINSTANCE, &ifcRelSpaceBoundaryInstance);
			(*pChild) = CreateTreeItem__IFCINSTANCE_spaceBoundary(treeItem, ifcModel, ifcRelSpaceBoundaryInstance);
			(*pChild)->child = CreateTreeItem__GEOMETRY((*pChild));
			int_t ifcObjectInstance = 0;
			sdaiGetAttrBN(ifcRelSpaceBoundaryInstance, (char*)L"RelatedBuildingElement", sdaiINSTANCE, &ifcObjectInstance);
			if (ifcObjectInstance) {
				(*pChild)->child->next = CreateTreeItem__SPACEBOUNDARIES((*pChild));
				(*pChild)->child->next->child = CreateTreeItem__IFCINSTANCE_spaceBoundary((*pChild)->child->next, ifcModel, ifcObjectInstance);
				(*pChild)->child->next->child->child = CreateTreeItem__GEOMETRY((*pChild)->child->next->child);
				(*pChild)->child->next->child->child->next = CreateTreeItem__PROPERTIES((*pChild)->child->next->child);
			}
			pChild = &(*pChild)->next;
		}
		return	treeItem;
	}
	return	nullptr;
}
STRUCT_TREE_ITEM** CreateTreeItem_ifcSpaceBoundary(int_t ifcModel, int_t ifcObjectInstance, STRUCT_TREE_ITEM *parent, STRUCT_TREE_ITEM	**pChild)
{
	if (sdaiGetInstanceType(ifcObjectInstance) == ifcSpace_TYPE) {
		(*pChild) = CreateTreeItem__IFCINSTANCE_spaceBoundary(parent, ifcModel, ifcObjectInstance);
		(*pChild)->child = CreateTreeItem__GEOMETRY((*pChild));
		(*pChild)->child->next = CreateTreeItem__PROPERTIES((*pChild));
		(*pChild)->child->next->next = CreateTreeItem_ifcSpaceBoundary_SPACE(ifcModel, ifcObjectInstance, (*pChild));
		pChild = &(*pChild)->next;
	}

	pChild = CreateTreeItem_ifcSpaceBoundaryDecomposedBy(ifcModel, ifcObjectInstance, parent, pChild);
	pChild = CreateTreeItem_ifcSpaceBoundaryContains(ifcModel, ifcObjectInstance, parent, pChild);

	return	pChild;
}
STRUCT_TREE_ITEM** CreateTreeItem_ifcSpaceBoundaryContains(int_t ifcModel, int_t ifcObjectInstance, STRUCT_TREE_ITEM *parent, STRUCT_TREE_ITEM** pChild)
{
	int_t *ifcRelContainedInSpatialStructureInstances = nullptr;
	sdaiGetAttrBN(ifcObjectInstance, (char*)L"ContainsElements", sdaiAGGR, &ifcRelContainedInSpatialStructureInstances);
	int_t ifcRelContainedInSpatialStructureInstancesCnt = sdaiGetMemberCount(ifcRelContainedInSpatialStructureInstances);

	for (int_t i = 0; i < ifcRelContainedInSpatialStructureInstancesCnt; ++i)
	{
		int_t ifcRelContainedInSpatialStructureInstance = 0;
		engiGetAggrElement(ifcRelContainedInSpatialStructureInstances, i, sdaiINSTANCE, &ifcRelContainedInSpatialStructureInstance);
		if (sdaiGetInstanceType(ifcRelContainedInSpatialStructureInstance) == ifcRelContainedInSpatialStructure_TYPE)
		{
			int_t	* ifcObjectInstances = nullptr;
			sdaiGetAttrBN(ifcRelContainedInSpatialStructureInstance, (char*)L"RelatedElements", sdaiAGGR, &ifcObjectInstances);
			int_t	ifcObjectInstancesCnt = sdaiGetMemberCount(ifcObjectInstances);
			if (ifcObjectInstancesCnt)
			{
				for (int_t k = 0; k < ifcObjectInstancesCnt; ++k)
				{
					ifcObjectInstance = 0;
					engiGetAggrElement(ifcObjectInstances, k, sdaiINSTANCE, &ifcObjectInstance);

					pChild = CreateTreeItem_ifcSpaceBoundary(ifcModel, ifcObjectInstance, parent, pChild);
				}
			}
		}
		else
		{
			_ASSERT(false);
		}
	}
	return	pChild;
}
STRUCT_TREE_ITEM** CreateTreeItem_ifcSpaceBoundaryDecomposedBy(int_t ifcModel, int_t ifcObjectInstance, STRUCT_TREE_ITEM *parent, STRUCT_TREE_ITEM	**pChild)
{
	int_t *ifcRelDecomposesInstances = nullptr, ifcRelDecomposesInstancesCnt;
	sdaiGetAttrBN(ifcObjectInstance, (char*)L"IsDecomposedBy", sdaiAGGR, &ifcRelDecomposesInstances);
	if (ifcRelDecomposesInstances)
	{
		ifcRelDecomposesInstancesCnt = sdaiGetMemberCount(ifcRelDecomposesInstances);
		for (int_t j = 0; j < ifcRelDecomposesInstancesCnt; ++j) {
			int_t ifcRelDecomposesInstance = 0;
			engiGetAggrElement(ifcRelDecomposesInstances, j, sdaiINSTANCE, &ifcRelDecomposesInstance);
			if (sdaiGetInstanceType(ifcRelDecomposesInstance) == ifcRelAggregates_TYPE) {
				int_t	* ifcObjectInstances = nullptr;
				sdaiGetAttrBN(ifcRelDecomposesInstance, (char*)L"RelatedObjects", sdaiAGGR, &ifcObjectInstances);

				int_t	ifcObjectInstancesCnt = sdaiGetMemberCount(ifcObjectInstances);
				if (ifcObjectInstancesCnt) {
					for (int_t k = 0; k < ifcObjectInstancesCnt; ++k) {
						ifcObjectInstance = 0;
						engiGetAggrElement(ifcObjectInstances, k, sdaiINSTANCE, &ifcObjectInstance);

						pChild = CreateTreeItem_ifcSpaceBoundary(ifcModel, ifcObjectInstance, parent, pChild);
					}
				}
			}
			else {
				_ASSERT(false);
			}
		}
	}
	return	pChild;
}
void CreateTreeItem_ifcSpaceBoundary(int_t ifcModel)
{
	ifcSpace_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCSPACE");
	int_t noIfcSpaceBoundary = sdaiGetMemberCount(sdaiGetEntityExtentBN(ifcModel, (char*)L"IFCRELSPACEBOUNDARY"));
	cout << "noIfcSpaceBoundary:" << noIfcSpaceBoundary << endl;

	if (noIfcSpaceBoundary)
	{
		STRUCT_TREE_ITEM *treeItemSpaceBoundaries = CreateTreeItem__SPACEBOUNDARIES(nullptr),
			             **treeItemChild = &treeItemSpaceBoundaries->child;

		topSpaceBoundaryTreeItem = treeItemSpaceBoundaries;
		treeItemSpaceBoundaries->next = topTreeItem;
		topTreeItem = treeItemSpaceBoundaries;
		int_t *ifcBuildingStoreyInstanceSet = sdaiGetEntityExtentBN(ifcModel, (char*)L"IFCBUILDINGSTOREY"),
			  ifcBuildingStoreyInstanceSetCnt = sdaiGetMemberCount(ifcBuildingStoreyInstanceSet);
		cout << "noIfcBuildingStoreyInstanceSet" << ifcBuildingStoreyInstanceSetCnt << endl;

		for (int_t i = 0; i < ifcBuildingStoreyInstanceSetCnt; i++) 
		{
			int_t ifcBuildingStoreyInstance = 0;
			engiGetAggrElement(ifcBuildingStoreyInstanceSet, i, sdaiINSTANCE, &ifcBuildingStoreyInstance);
			(*treeItemChild) = CreateTreeItem__IFCINSTANCE_spaceBoundary(treeItemSpaceBoundaries, ifcModel, ifcBuildingStoreyInstance);
			CreateTreeItem_ifcSpaceBoundary(ifcModel, ifcBuildingStoreyInstance, (*treeItemChild), &(*treeItemChild)->child);
			treeItemChild = &(*treeItemChild)->next;
		}
	}
}

//CreateTreeItem_nonReferencedIfcItems
STRUCT_TREE_ITEM* CreateTreeItem__NOTREFERENCED(STRUCT_TREE_ITEM *parent)
{
	STRUCT_TREE_ITEM_NOTREFERENCED	*treeItem = new STRUCT_TREE_ITEM_NOTREFERENCED;
	treeItem->type = TREE_ITEM_NOTREFERENCED;
	InitTreeItemSelectable(treeItem, parent);
	treeItem->nameBuffer = L"not referenced";
	_ASSERT(treeItem->parent == nullptr);
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__IFCENTITY(STRUCT_TREE_ITEM *parent, int_t ifcModel, int_t	ifcEntity)
{
	STRUCT_TREE_ITEM_IFCENTITY	*treeItem = new STRUCT_TREE_ITEM_IFCENTITY;
	treeItem->type = TREE_ITEM_IFCENTITY;
	InitTreeItemSelectable(treeItem, parent);

	//	Init Tree Item IfcEntity
	treeItem->ifcModel = ifcModel;
	treeItem->ifcEntity = ifcEntity;
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__IFCINSTANCE_nonReferenced(STRUCT_TREE_ITEM *parent, int_t	ifcModel, int_t	ifcInstance)
{
	STRUCT_TREE_ITEM_IFCINSTANCE	* treeItem = new STRUCT_TREE_ITEM_IFCINSTANCE;
	treeItem->type = TREE_ITEM_IFCINSTANCE;
	InitTreeItemSelectable(treeItem, parent);

	//	Init Tree Item IfcInstance
	treeItem->ifcModel = ifcModel;
	treeItem->ifcInstance = ifcInstance;
	treeItem->ifcObject = allIfcObjects[ifcInstance];
	if (treeItem->ifcObject)
	{
		_ASSERT(treeItem->ifcObject->treeItemNonReferenced == nullptr);
		treeItem->ifcObject->treeItemNonReferenced = treeItem;
	}
	return	treeItem;
}
void CreateTreeItem_nonReferencedIfcItems(int_t	ifcModel)
{
	int_t nonReferencedItemCnt = 0;
	STRUCT__IFC__OBJECT	*ifcObject = ifcObjectLinkedList;
	while (ifcObject)
	{
		_ASSERT(ifcObject->ifcInstance);
		if (ifcObject->treeItemModel == nullptr && ifcObject->treeItemSpaceBoundary == nullptr)
		{
			nonReferencedItemCnt++;
		}
		ifcObject = ifcObject->next;
	}
	cout << "noRefrencedItemCnt:" << nonReferencedItemCnt << endl;
	if (nonReferencedItemCnt)
	{
		wchar_t	buffer[100];
		_itow_s((int32_t)nonReferencedItemCnt, buffer, 100, 10);
		topNonReferencedTreeItem = CreateTreeItem__NOTREFERENCED(nullptr);// , L"non-referenced instances");
		STRUCT_TREE_ITEM **pChild = &topNonReferencedTreeItem->child;

		topNonReferencedTreeItem->next = topTreeItem;
		topTreeItem = topNonReferencedTreeItem;

		ifcObject = ifcObjectLinkedList;
		while (ifcObject)
		{
			int_t ifcEntity = ifcObject->ifcEntity, cnt = 0;
			STRUCT__IFC__OBJECT	* ifcObjectCnt = ifcObject;
			while (ifcObjectCnt  &&  ifcObjectCnt->ifcEntity == ifcEntity)
			{
				if (ifcObjectCnt->treeItemModel == nullptr && ifcObjectCnt->treeItemSpaceBoundary == nullptr)
				{
					cnt++;
				}
				ifcObjectCnt = ifcObjectCnt->next;
			}

			if (cnt)
			{
				(*pChild) = CreateTreeItem__IFCENTITY(topNonReferencedTreeItem, ifcModel, ifcEntity);
				STRUCT_TREE_ITEM **pSubChild = &(*pChild)->child;

				while (ifcObject  &&  ifcObject->ifcEntity == ifcEntity)
				{
					if (ifcObject->treeItemModel == nullptr && ifcObject->treeItemSpaceBoundary == nullptr)
					{
						(*pSubChild) = CreateTreeItem__IFCINSTANCE_nonReferenced((*pChild), ifcModel, ifcObject->ifcInstance);
						(*pSubChild)->child = CreateTreeItem__GEOMETRY((*pSubChild));
						(*pSubChild)->child->next = CreateTreeItem__PROPERTIES((*pSubChild));
						pSubChild = &(*pSubChild)->next;
					}
					ifcObject = ifcObject->next;
				}
				pChild = &(*pChild)->next;
			}
			else
			{
				ifcObject = ifcObjectCnt;
			}
		}
	}
}

//CreateTreeItem_ifcGroup
STRUCT_TREE_ITEM* CreateTreeItem__IFCINSTANCE_group(STRUCT_TREE_ITEM *parent, int_t	ifcModel, int_t	ifcInstance)
{
	STRUCT_TREE_ITEM_IFCINSTANCE	* treeItem = new STRUCT_TREE_ITEM_IFCINSTANCE;
	treeItem->type = TREE_ITEM_IFCINSTANCE;
	InitTreeItemSelectable(treeItem, parent);

	//	Init Tree Item IfcInstance
	treeItem->ifcModel = ifcModel;
	treeItem->ifcInstance = ifcInstance;

	treeItem->ifcObject = allIfcObjects[ifcInstance];
	if (treeItem->ifcObject)
	{
		//		treeItem->ifcObject->treeItem... = treeItem;
	}
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__GROUPEDBY(STRUCT_TREE_ITEM *parent)
{
	STRUCT_TREE_ITEM_GROUPEDBY	*treeItem = new STRUCT_TREE_ITEM_GROUPEDBY;
	treeItem->type = TREE_ITEM_GROUPEDBY;
	InitTreeItemSelectable(treeItem, parent);
	treeItem->nameBuffer = L"grouped by";
	return	treeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem_ifcGroup(int_t ifcModel, int_t	ifcGroupInstance, STRUCT_TREE_ITEM	*parent)
{
	STRUCT_TREE_ITEM *treeItem = CreateTreeItem__IFCINSTANCE_group(parent, ifcModel, ifcGroupInstance);
	int_t ifcRelAssignsToGroupInstance = 0;
	sdaiGetAttrBN(ifcGroupInstance, (char*)L"IsGroupedBy", sdaiINSTANCE, &ifcRelAssignsToGroupInstance);
	if (ifcRelAssignsToGroupInstance)
	{
		int_t *ifcObjectDefinitionInstanceSet = nullptr;
		sdaiGetAttrBN(ifcRelAssignsToGroupInstance, (char*)L"RelatedObjects", sdaiAGGR, &ifcObjectDefinitionInstanceSet);

		treeItem->child = CreateTreeItem__GROUPEDBY(treeItem);
		STRUCT_TREE_ITEM **pTreeItemGroups = &treeItem->child->child;
		int_t ifcObjectDefinitionInstanceSetCnt = sdaiGetMemberCount(ifcObjectDefinitionInstanceSet);
		if (ifcObjectDefinitionInstanceSetCnt)
		{
			int_t *ifcInstanceArray = new int_t[ifcObjectDefinitionInstanceSetCnt],
				*ifcInstanceEntityArray = new int_t[ifcObjectDefinitionInstanceSetCnt];
			for (int_t i = 0; i < ifcObjectDefinitionInstanceSetCnt; i++) {
				int_t	ifcObjectDefinitionInstance = 0;
				engiGetAggrElement(ifcObjectDefinitionInstanceSet, i, sdaiINSTANCE, &ifcObjectDefinitionInstance);

				ifcInstanceArray[i] = ifcObjectDefinitionInstance;
				ifcInstanceEntityArray[i] = sdaiGetInstanceType(ifcObjectDefinitionInstance);
				_ASSERT(ifcInstanceEntityArray[i]);
			}

			for (int_t i = 0; i < ifcObjectDefinitionInstanceSetCnt; i++)
			{
				if (ifcInstanceEntityArray[i])
				{
					int_t j = i, ifcInstanceEntity = ifcInstanceEntityArray[i];
					(*pTreeItemGroups) = CreateTreeItem__IFCENTITY(treeItem->child, ifcModel, ifcInstanceEntity);
					STRUCT_TREE_ITEM	** pTreeItemInstances = &(*pTreeItemGroups)->child;
					while (j < ifcObjectDefinitionInstanceSetCnt)
					{
						if (ifcInstanceEntityArray[j] == ifcInstanceEntity)
						{
							(*pTreeItemInstances) = CreateTreeItem__IFCINSTANCE_group((*pTreeItemGroups), ifcModel, ifcInstanceArray[j]);
							(*pTreeItemInstances)->child = CreateTreeItem__GEOMETRY((*pTreeItemInstances));
							(*pTreeItemInstances)->child->next = CreateTreeItem__PROPERTIES((*pTreeItemInstances));
							pTreeItemInstances = &(*pTreeItemInstances)->next;
							ifcInstanceEntityArray[j] = 0;
						}
						j++;
					}
					pTreeItemGroups = &(*pTreeItemGroups)->next;
				}
				int_t ifcObjectDefinitionInstance = 0;
				engiGetAggrElement(ifcObjectDefinitionInstanceSet, i, sdaiINSTANCE, &ifcObjectDefinitionInstance);
				ifcInstanceArray[i] = ifcObjectDefinitionInstance;
				ifcInstanceEntityArray[i] = sdaiGetInstanceType(ifcObjectDefinitionInstance);
				_ASSERT(ifcInstanceEntityArray[i]);
			}
			delete[] ifcInstanceArray;
			delete[] ifcInstanceEntityArray;
		}
	}
	else
	{
		//_ASSERT(false);
	}
	return	treeItem;
}
STRUCT_TREE_ITEM** CreateTreeItem_ifcGroupRecursively(int_t	ifcModel, int_t	ifcParentEntity, STRUCT_TREE_ITEM **pTreeItem)
{
	int_t *ifcEntityExtend = sdaiGetEntityExtent(ifcModel, ifcParentEntity),
		cnt = sdaiGetMemberCount(ifcEntityExtend);
	for (int_t i = 0; i < cnt; ++i)
	{
		int_t ifcParentEntityInstance = 0;
		engiGetAggrElement(ifcEntityExtend, i, sdaiINSTANCE, &ifcParentEntityInstance);
		(*pTreeItem) = CreateTreeItem_ifcGroup(ifcModel, ifcParentEntityInstance, nullptr);
		_ASSERT((*pTreeItem)->next == nullptr);
		pTreeItem = &(*pTreeItem)->next;
	}

	cnt = engiGetEntityCount(ifcModel);
	for (int_t i = 0; i < cnt; i++)
	{
		int_t ifcEntity = engiGetEntityElement(ifcModel, i);
		if (engiGetEntityParent(ifcEntity) == ifcParentEntity)
		{
			pTreeItem = CreateTreeItem_ifcGroupRecursively(ifcModel, ifcEntity, pTreeItem);
		}
	}
	return	pTreeItem;
}
STRUCT_TREE_ITEM* CreateTreeItem__GROUPS(STRUCT_TREE_ITEM *parent)
{
	STRUCT_TREE_ITEM_GROUPS	* treeItem = new STRUCT_TREE_ITEM_GROUPS;
	treeItem->type = TREE_ITEM_GROUPS;
	InitTreeItemSelectable(treeItem, parent);
	treeItem->nameBuffer = L"groups";
	return	treeItem;
}
void CreateTreeItem_ifcGroup(int_t ifcModel)
{
	STRUCT_TREE_ITEM *treeItem = nullptr,
		             **pTreeItem = &treeItem;
	CreateTreeItem_ifcGroupRecursively(ifcModel, sdaiGetEntity(ifcModel, (char*)L"IFCGROUP"), pTreeItem);
	cout << "ifcGroupTreeItem是否为空:" << (&pTreeItem==nullptr)<< endl;
	if (treeItem)
	{
		STRUCT_TREE_ITEM *tTreeItem = CreateTreeItem__GROUPS(nullptr);
		topGroupTreeItem = tTreeItem;
		tTreeItem->child = treeItem;
		while (treeItem)
		{
			_ASSERT(treeItem->parent == nullptr);
			treeItem->parent = tTreeItem;
			treeItem = treeItem->next;
		}
		tTreeItem->next = topTreeItem;
		topTreeItem = tTreeItem;
	}
}


//GetUnits
bool equalStr(wchar_t * txtI, wchar_t * txtII)
{
	int_t i = 0;
	if (txtI && txtII) {
		while (txtI[i]) {
			if (txtI[i] != txtII[i]) {
				return	false;
			}
			i++;
		}
		if (txtII[i]) {
			return	false;
		}
	}
	else if (txtI || txtII) {
		return	false;
	}
	return	true;
}
void UnitAddUnitType(STRUCT__SIUNIT * unit, wchar_t * unitType)
{
	//
	//	unitType
	//
	if (equalStr(unitType, L".ABSORBEDDOSEUNIT.")) {
		unit->type = ABSORBEDDOSEUNIT;
		unit->unitType = L"ABSORBEDDOSEUNIT";
	}
	else if (equalStr(unitType, L".AREAUNIT.")) {
		unit->type = AREAUNIT;
		unit->unitType = L"AREAUNIT";
	}
	else if (equalStr(unitType, L".DOSEEQUIVALENTUNIT.")) {
		unit->type = DOSEEQUIVALENTUNIT;
		unit->unitType = L"DOSEEQUIVALENTUNIT";
	}
	else if (equalStr(unitType, L".ELECTRICCAPACITANCEUNIT.")) {
		unit->type = ELECTRICCAPACITANCEUNIT;
		unit->unitType = L"ELECTRICCAPACITANCEUNIT";
	}
	else if (equalStr(unitType, L".ELECTRICCHARGEUNIT.")) {
		unit->type = ELECTRICCHARGEUNIT;
		unit->unitType = L"ELECTRICCHARGEUNIT";
	}
	else if (equalStr(unitType, L".ELECTRICCONDUCTANCEUNIT.")) {
		unit->type = ELECTRICCONDUCTANCEUNIT;
		unit->unitType = L"ELECTRICCONDUCTANCEUNIT";
	}
	else if (equalStr(unitType, L".ELECTRICCURRENTUNIT.")) {
		unit->type = ELECTRICCURRENTUNIT;
		unit->unitType = L"ELECTRICCURRENTUNIT";
	}
	else if (equalStr(unitType, L".ELECTRICRESISTANCEUNIT.")) {
		unit->type = ELECTRICRESISTANCEUNIT;
		unit->unitType = L"ELECTRICRESISTANCEUNIT";
	}
	else if (equalStr(unitType, L".ELECTRICVOLTAGEUNIT.")) {
		unit->type = ELECTRICVOLTAGEUNIT;
		unit->unitType = L"ELECTRICVOLTAGEUNIT";
	}
	else if (equalStr(unitType, L".ENERGYUNIT.")) {
		unit->type = ENERGYUNIT;
		unit->unitType = L"ENERGYUNIT";
	}
	else if (equalStr(unitType, L".FORCEUNIT.")) {
		unit->type = FORCEUNIT;
		unit->unitType = L"FORCEUNIT";
	}
	else if (equalStr(unitType, L".FREQUENCYUNIT.")) {
		unit->type = FREQUENCYUNIT;
		unit->unitType = L"FREQUENCYUNIT";
	}
	else if (equalStr(unitType, L".ILLUMINANCEUNIT.")) {
		unit->type = ILLUMINANCEUNIT;
		unit->unitType = L"ILLUMINANCEUNIT";
	}
	else if (equalStr(unitType, L".INDUCTANCEUNIT.")) {
		unit->type = INDUCTANCEUNIT;
		unit->unitType = L"INDUCTANCEUNIT";
	}
	else if (equalStr(unitType, L".LENGTHUNIT.")) {
		unit->type = LENGTHUNIT;
		unit->unitType = L"LENGTHUNIT";
	}
	else if (equalStr(unitType, L".LUMINOUSFLUXUNIT.")) {
		unit->type = LUMINOUSFLUXUNIT;
		unit->unitType = L"LUMINOUSFLUXUNIT";
	}
	else if (equalStr(unitType, L".LUMINOUSINTENSITYUNIT.")) {
		unit->type = LUMINOUSINTENSITYUNIT;
		unit->unitType = L"LUMINOUSINTENSITYUNIT";
	}
	else if (equalStr(unitType, L".MAGNETICFLUXDENSITYUNIT.")) {
		unit->type = MAGNETICFLUXDENSITYUNIT;
		unit->unitType = L"MAGNETICFLUXDENSITYUNIT";
	}
	else if (equalStr(unitType, L".MAGNETICFLUXUNIT.")) {
		unit->type = MAGNETICFLUXUNIT;
		unit->unitType = L"MAGNETICFLUXUNIT";
	}
	else if (equalStr(unitType, L".MASSUNIT.")) {
		unit->type = MASSUNIT;
		unit->unitType = L"MASSUNIT";
	}
	else if (equalStr(unitType, L".PLANEANGLEUNIT.")) {
		unit->type = PLANEANGLEUNIT;
		unit->unitType = L"PLANEANGLEUNIT";
	}
	else if (equalStr(unitType, L".POWERUNIT.")) {
		unit->type = POWERUNIT;
		unit->unitType = L"POWERUNIT";
	}
	else if (equalStr(unitType, L".PRESSUREUNIT.")) {
		unit->type = PRESSUREUNIT;
		unit->unitType = L"PRESSUREUNIT";
	}
	else if (equalStr(unitType, L".RADIOACTIVITYUNIT.")) {
		unit->type = RADIOACTIVITYUNIT;
		unit->unitType = L"RADIOACTIVITYUNIT";
	}
	else if (equalStr(unitType, L".SOLIDANGLEUNIT.")) {
		unit->type = SOLIDANGLEUNIT;
		unit->unitType = L"SOLIDANGLEUNIT";
	}
	else if (equalStr(unitType, L".THERMODYNAMICTEMPERATUREUNIT.")) {
		unit->type = THERMODYNAMICTEMPERATUREUNIT;
		unit->unitType = L"THERMODYNAMICTEMPERATUREUNIT";
	}
	else if (equalStr(unitType, L".TIMEUNIT.")) {
		unit->type = TIMEUNIT;
		unit->unitType = L"TIMEUNIT";
	}
	else if (equalStr(unitType, L".VOLUMEUNIT.")) {
		unit->type = VOLUMEUNIT;
		unit->unitType = L"VOLUMEUNIT";
	}
	else if (equalStr(unitType, L".USERDEFINED.")) {
		unit->type = USERDEFINED;
		unit->unitType = L"USERDEFINED";
	}
	else {
		//_ASSERT(false);
	}
}
void UnitAddPrefix(STRUCT__SIUNIT * unit, wchar_t * prefix)
{
	//
	//	prefix
	//
	if (equalStr(prefix, L".EXA.")) {
		unit->prefix = L"Exa";
	}
	else if (equalStr(prefix, L".PETA.")) {
		unit->prefix = L"Peta";
	}
	else if (equalStr(prefix, L".TERA.")) {
		unit->prefix = L"Tera";
	}
	else if (equalStr(prefix, L".GIGA.")) {
		unit->prefix = L"Giga";
	}
	else if (equalStr(prefix, L".MEGA.")) {
		unit->prefix = L"Mega";
	}
	else if (equalStr(prefix, L".KILO.")) {
		unit->prefix = L"Kilo";
	}
	else if (equalStr(prefix, L".HECTO.")) {
		unit->prefix = L"Hecto";
	}
	else if (equalStr(prefix, L".DECA.")) {
		unit->prefix = L"Deca";
	}
	else if (equalStr(prefix, L".DECI.")) {
		unit->prefix = L"Deci";
	}
	else if (equalStr(prefix, L".CENTI.")) {
		unit->prefix = L"Centi";
	}
	else if (equalStr(prefix, L".MILLI.")) {
		unit->prefix = L"Milli";
	}
	else if (equalStr(prefix, L".MICRO.")) {
		unit->prefix = L"Micro";
	}
	else if (equalStr(prefix, L".NANO.")) {
		unit->prefix = L"Nano";
	}
	else if (equalStr(prefix, L".PICO.")) {
		unit->prefix = L"Pico";
	}
	else if (equalStr(prefix, L".FEMTO.")) {
		unit->prefix = L"Femto";
	}
	else if (equalStr(prefix, L".ATTO.")) {
		unit->prefix = L"Atto";
	}
	else {
		_ASSERT(prefix == 0);
	}
}
void UnitAddName(STRUCT__SIUNIT * unit, wchar_t * name)
{
	//
	//	name
	//
	if (equalStr(name, L".AMPERE.")) {
		unit->name = L"Ampere";
	}
	else if (equalStr(name, L".BECQUEREL.")) {
		unit->name = L"Becquerel";
	}
	else if (equalStr(name, L".CANDELA.")) {
		unit->name = L"Candela";
	}
	else if (equalStr(name, L".COULOMB.")) {
		unit->name = L"Coulomb";
	}
	else if (equalStr(name, L".CUBIC_METRE.")) {
		unit->name = L"Cubic Metre";
	}
	else if (equalStr(name, L".DEGREE_CELSIUS.")) {
		unit->name = L"Degree Celcius";
	}
	else if (equalStr(name, L".FARAD.")) {
		unit->name = L"Farad";
	}
	else if (equalStr(name, L".GRAM.")) {
		unit->name = L"Gram";
	}
	else if (equalStr(name, L".GRAY.")) {
		unit->name = L"Gray";
	}
	else if (equalStr(name, L".HENRY.")) {
		unit->name = L"Henry";
	}
	else if (equalStr(name, L".HERTZ.")) {
		unit->name = L"Hertz";
	}
	else if (equalStr(name, L".JOULE.")) {
		unit->name = L"Joule";
	}
	else if (equalStr(name, L".KELVIN.")) {
		unit->name = L"Kelvin";
	}
	else if (equalStr(name, L".LUMEN.")) {
		unit->name = L"Lumen";
	}
	else if (equalStr(name, L".LUX.")) {
		unit->name = L"Lux";
	}
	else if (equalStr(name, L".METRE.")) {
		unit->name = L"Metre";
	}
	else if (equalStr(name, L".MOLE.")) {
		unit->name = L"Mole";
	}
	else if (equalStr(name, L".NEWTON.")) {
		unit->name = L"Newton";
	}
	else if (equalStr(name, L".OHM.")) {
		unit->name = L"Ohm";
	}
	else if (equalStr(name, L".PASCAL.")) {
		unit->name = L"Pascal";
	}
	else if (equalStr(name, L".RADIAN.")) {
		unit->name = L"Radian";
	}
	else if (equalStr(name, L".SECOND.")) {
		unit->name = L"Second";
	}
	else if (equalStr(name, L".SIEMENS.")) {
		unit->name = L"Siemens";
	}
	else if (equalStr(name, L".SIEVERT.")) {
		unit->name = L"Sievert";
	}
	else if (equalStr(name, L".SQUARE_METRE.")) {
		unit->name = L"Square Metre";
	}
	else if (equalStr(name, L".STERADIAN.")) {
		unit->name = L"Steradian";
	}
	else if (equalStr(name, L".TESLA.")) {
		unit->name = L"Tesla";
	}
	else if (equalStr(name, L".VOLT.")) {
		unit->name = L"Volt";
	}
	else if (equalStr(name, L".WATT.")) {
		unit->name = L"Watt";
	}
	else if (equalStr(name, L".WEBER.")) {
		unit->name = L"Weber";
	}
	else {
		_ASSERT(false);
	}
}
STRUCT__SIUNIT	*GetUnits(int_t ifcModel, int_t ifcProjectInstance)
{
	STRUCT__SIUNIT	* firstUnit = 0;

	int_t	ifcUnitAssignmentInstance = 0;
	sdaiGetAttrBN(ifcProjectInstance, (char*)L"UnitsInContext", sdaiINSTANCE, &ifcUnitAssignmentInstance);

	int_t	ifcConversianBasedUnit_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCCONVERSIONBASEDUNIT"),
		ifcSIUnit_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCSIUNIT");

	int_t	* unit_set = 0, unit_cnt, i = 0;

	sdaiGetAttrBN(ifcUnitAssignmentInstance, (char*)L"Units", sdaiAGGR, &unit_set);

	unit_cnt = sdaiGetMemberCount(unit_set);
	for (i = 0; i < unit_cnt; ++i) {
		int_t	ifcUnitInstance = 0;
		engiGetAggrElement(unit_set, i, sdaiINSTANCE, &ifcUnitInstance);
		if (sdaiGetInstanceType(ifcUnitInstance) == ifcConversianBasedUnit_TYPE) {
			STRUCT__SIUNIT	* unit = new STRUCT__SIUNIT();
			unit->unitType = 0;
			unit->prefix = 0;
			unit->name = 0;
			unit->next = firstUnit;
			firstUnit = unit;

			int_t	ifcMeasureWithUnitInstance = 0;

			sdaiGetAttrBN(ifcUnitInstance, (char*)L"ConversionFactor", sdaiINSTANCE, &ifcMeasureWithUnitInstance);

			if (ifcMeasureWithUnitInstance) {
				int_t	ifcSIUnitInstance = 0;

				int_t  * adb = 0;
				sdaiGetAttrBN(ifcMeasureWithUnitInstance, (char*)L"UnitComponent", sdaiINSTANCE, &ifcSIUnitInstance);
				sdaiGetAttrBN(ifcMeasureWithUnitInstance, (char*)L"ValueComponent", sdaiADB, &adb);

				double value = 0;
				sdaiGetADBValue(adb, sdaiREAL, &value);

				if (sdaiGetInstanceType(ifcSIUnitInstance) == ifcSIUnit_TYPE) {
					wchar_t	* unitType = 0, *prefix = 0, *name = 0;

					sdaiGetAttrBN(ifcSIUnitInstance, (char*)L"UnitType", sdaiUNICODE, &unitType);
					sdaiGetAttrBN(ifcSIUnitInstance, (char*)L"Prefix", sdaiUNICODE, &prefix);
					sdaiGetAttrBN(ifcSIUnitInstance, (char*)L"Name", sdaiUNICODE, &name);

					UnitAddUnitType(unit, unitType);
					UnitAddPrefix(unit, prefix);
					UnitAddName(unit, name);
				}
				else {
					_ASSERT(false);
				}
			}
			else {
				_ASSERT(false);
			}
		}
		else if (sdaiGetInstanceType(ifcUnitInstance) == ifcSIUnit_TYPE) {
			STRUCT__SIUNIT	* unit = new STRUCT__SIUNIT();
			unit->unitType = 0;
			unit->prefix = 0;
			unit->name = 0;
			unit->next = firstUnit;
			firstUnit = unit;

			wchar_t	* unitType = 0, *prefix = 0, *name = 0;

			sdaiGetAttrBN(ifcUnitInstance, (char*)L"UnitType", sdaiUNICODE, &unitType);
			sdaiGetAttrBN(ifcUnitInstance, (char*)L"Prefix", sdaiUNICODE, &prefix);
			sdaiGetAttrBN(ifcUnitInstance, (char*)L"Name", sdaiUNICODE, &name);

			UnitAddUnitType(unit, unitType);
			UnitAddPrefix(unit, prefix);
			UnitAddName(unit, name);
		}
		else {
			///////////////////			ASSERT(false);
		}
	}

	return	firstUnit;
}


//CreateIfcInstanceProperties
wchar_t	* copyStr(wchar_t * txt)
{
	if (txt) {
		int_t	i = 0;
		while (txt[i]) { i++; }
		wchar_t	* rValue = new wchar_t[i + 1];
		i = 0;
		while (txt[i]) {
			rValue[i] = txt[i];
			i++;
		}
		rValue[i] = 0;

		return	rValue;
	}
	else {
		return	0;
	}
}
STRUCT__PROPERTY__SET* CreateIfcPropertySet(int_t ifcInstance, wchar_t * name, wchar_t * description)
{
	STRUCT__PROPERTY__SET	* ifcPropertySet = new STRUCT__PROPERTY__SET;

	ifcPropertySet->structType = STRUCT_TYPE_PROPERTY_SET;
	ifcPropertySet->ifcInstance = ifcInstance;
	ifcPropertySet->name = copyStr(name);
	ifcPropertySet->description = copyStr(description);

	ifcPropertySet->hTreeItem = 0;

	ifcPropertySet->nameBuffer = new wchar_t[512];

	ifcPropertySet->properties = 0;
	ifcPropertySet->next = 0;

	return	ifcPropertySet;
}
STRUCT__PROPERTY* CreateIfcProperty(int_t ifcInstance, wchar_t * name, wchar_t * description)
{
	STRUCT__PROPERTY	* ifcProperty = new STRUCT__PROPERTY;

	ifcProperty->structType = STRUCT_TYPE_PROPERTY;
	ifcProperty->ifcInstance = ifcInstance;
	ifcProperty->name = copyStr(name);
	ifcProperty->description = copyStr(description);

	ifcProperty->nominalValue = 0;
	ifcProperty->lengthValue = 0;
	ifcProperty->areaValue = 0;
	ifcProperty->volumeValue = 0;
	ifcProperty->countValue = 0;
	ifcProperty->weigthValue = 0;
	ifcProperty->timeValue = 0;
	ifcProperty->unit = 0;

	ifcProperty->hTreeItem = 0;
	ifcProperty->nameBuffer = new wchar_t[512 + 1];
	ifcProperty->next = 0;

	return	ifcProperty;
}
wchar_t	* GetUnit(STRUCT__SIUNIT * units, wchar_t * unitType)
{
	STRUCT__SIUNIT	* unit = units;

	while (unit) {
		if (equalStr(unit->unitType, unitType)) {
			int_t i = 0, j = 0;
			if (unit->prefix) {
				while (unit->prefix[i]) { i++; }
				i++;
			}
			if (unit->name) {
				while (unit->name[j]) { j++; }
			}

			wchar_t	* rValue = new wchar_t[i + j + 1];

			i = 0;
			if (unit->prefix) {
				while (unit->prefix[i]) { rValue[i++] = unit->prefix[i]; }
				rValue[i++] = ' ';
			}
			j = 0;
			if (unit->name) {
				while (unit->name[j]) { rValue[i + j++] = unit->name[j]; }
				rValue[i + j] = 0;
			}
			return	rValue;
		}
		unit = unit->next;
	}

	return	0;
}
void CreateIfcPropertySingleValue(int_t ifcPropertySingleValue, STRUCT__PROPERTY * ifcProperty, STRUCT__SIUNIT * units)
{
	wchar_t	* nominalValue = 0,
		*unit = 0,
		*typePath = 0;
	int_t	* nominalValueADB = 0;

	sdaiGetAttrBN(ifcPropertySingleValue, (char*)L"NominalValue", sdaiUNICODE, &nominalValue);
	if (nominalValue) {
		sdaiGetAttrBN(ifcPropertySingleValue, (char*)L"NominalValue", sdaiADB, &nominalValueADB);
		typePath = (wchar_t*)sdaiGetADBTypePath(nominalValueADB, 0);
		sdaiGetAttrBN(ifcPropertySingleValue, (char*)L"Unit", sdaiUNICODE, &unit);

		if (unit == 0) {
			if (equalStr(typePath, L"IFCBOOLEAN")) {
			}
			else if (equalStr(typePath, L"IFCIDENTIFIER")) {
			}
			else if (equalStr(typePath, L"IFCINTEGER")) {
			}
			else if (equalStr(typePath, L"IFCLABEL")) {
			}
			else if (equalStr(typePath, L"IFCLOGICAL")) {
			}
			else if (equalStr(typePath, L"IFCTEXT")) {
			}
			else if (equalStr(typePath, L"IFCREAL")) {
			}
			else if (equalStr(typePath, L"IFCCOUNTMEASURE")) {
			}
			else if (equalStr(typePath, L"IFCPOSITIVERATIOMEASURE")) {
			}
			else if (equalStr(typePath, L"IFCVOLUMETRICFLOWRATEMEASURE")) 
			{
			}
			else if (equalStr(typePath, L"IFCABSORBEDDOSEMEASURE"))
			{
				unit = GetUnit(units, L"ABSORBEDDOSEUNIT");
			}
			else if (equalStr(typePath, L"IFCAMOUNTOFSUBSTANCEMEASURE")) {
				unit = GetUnit(units, L"AMOUNTOFSUBSTANCEUNIT");
			}
			else if (equalStr(typePath, L"IFCAREAMEASURE")) {
				unit = GetUnit(units, L"AREAUNIT");
			}
			else if (equalStr(typePath, L"IFCDOSEEQUIVALENTMEASURE")) {
				unit = GetUnit(units, L"DOSEEQUIVALENTUNIT");
			}
			else if (equalStr(typePath, L"IFCELECTRICCAPACITANCEMEASURE")) {
				unit = GetUnit(units, L"ELECTRICCAPACITANCEUNIT");
			}
			else if (equalStr(typePath, L"IFCELECTRICCHARGEMEASURE")) {
				unit = GetUnit(units, L"ELECTRICCHARGEUNIT");
			}
			else if (equalStr(typePath, L"IFCELECTRICCONDUCTANCEMEASURE")) {
				unit = GetUnit(units, L"ELECTRICCONDUCTANCEUNIT");
			}
			else if (equalStr(typePath, L"IFCELECTRICCURRENTMEASURE")) {
				unit = GetUnit(units, L"ELECTRICCURRENTUNIT");
			}
			else if (equalStr(typePath, L"IFCELECTRICRESISTANCEMEASURE")) {
				unit = GetUnit(units, L"ELECTRICRESISTANCEUNIT");
			}
			else if (equalStr(typePath, L"IFCELECTRICVOLTAGEMEASURE")) {
				unit = GetUnit(units, L"ELECTRICVOLTAGEUNIT");
			}
			else if (equalStr(typePath, L"IFCENERGYMEASURE")) {
				unit = GetUnit(units, L"ENERGYUNIT");
			}
			else if (equalStr(typePath, L"IFCFORCEMEASURE")) {
				unit = GetUnit(units, L"FORCEUNIT");
			}
			else if (equalStr(typePath, L"IFCFREQUENCYMEASURE")) {
				unit = GetUnit(units, L"FREQUENCYUNIT");
			}
			else if (equalStr(typePath, L"IFCILLUMINANCEMEASURE")) {
				unit = GetUnit(units, L"ILLUMINANCEUNIT");
			}
			else if (equalStr(typePath, L"IFCINDUCTANCEMEASURE")) {
				unit = GetUnit(units, L"INDUCTANCEUNIT");
			}
			else if (equalStr(typePath, L"IFCLENGTHMEASURE") || equalStr(typePath, L"IFCPOSITIVELENGTHMEASURE")) {
				unit = GetUnit(units, L"LENGTHUNIT");
			}
			else if (equalStr(typePath, L"IFCLUMINOUSFLUXMEASURE")) {
				unit = GetUnit(units, L"LUMINOUSFLUXUNIT");
			}
			else if (equalStr(typePath, L"IFCLUMINOUSINTENSITYMEASURE")) {
				unit = GetUnit(units, L"LUMINOUSINTENSITYUNIT");
			}
			else if (equalStr(typePath, L"IFCMAGNETICFLUXDENSITYMEASURE")) {
				unit = GetUnit(units, L"MAGNETICFLUXDENSITYUNIT");
			}
			else if (equalStr(typePath, L"IFCMAGNETICFLUXMEASURE")) {
				unit = GetUnit(units, L"MAGNETICFLUXUNIT");
			}
			else if (equalStr(typePath, L"IFCMASSMEASURE")) {
				unit = GetUnit(units, L"MASSUNIT");
			}
			else if (equalStr(typePath, L"IFCPLANEANGLEMEASURE")) {
				unit = GetUnit(units, L"PLANEANGLEUNIT");
			}
			else if (equalStr(typePath, L"IFPOWERCMEASURE")) {
				unit = GetUnit(units, L"POWERUNIT");
			}
			else if (equalStr(typePath, L"IFCPRESSUREMEASURE")) {
				unit = GetUnit(units, L"PRESSUREUNIT");
			}
			else if (equalStr(typePath, L"IFCRADIOACTIVITYMEASURE")) {
				unit = GetUnit(units, L"RADIOACTIVITYUNIT");
			}
			else if (equalStr(typePath, L"IFCSOLIDANGLEMEASURE")) {
				unit = GetUnit(units, L"SOLIDANGLEUNIT");
			}
			else if (equalStr(typePath, L"IFCTHERMODYNAMICTEMPERATUREMEASURE")) {
				unit = GetUnit(units, L"THERMODYNAMICTEMPERATUREUNIT");
			}
			else if (equalStr(typePath, L"IFCTIMEMEASURE")) {
				unit = GetUnit(units, L"TIMEUNIT");
			}
			else if (equalStr(typePath, L"IFCVOLUMEMEASURE")) {
				unit = GetUnit(units, L"VOLUMEUNIT");
			}
			else if (equalStr(typePath, L"IFCUSERDEFINEDMEASURE")) {
				unit = GetUnit(units, L"USERDEFINED");
			}
			else if (equalStr(typePath, L"IFCTHERMALTRANSMITTANCEMEASURE")) {
				unit = GetUnit(units, L"???");
			}
			else {
				_ASSERT(false);
			}
		}
		else {
			_ASSERT(false);
		}
		ifcProperty->nominalValue = copyStr(nominalValue);
		ifcProperty->unit = copyStr(unit);
	}
	else {
		ifcProperty->nominalValue = 0;
		ifcProperty->unit = 0;
	}
}
void CreateIfcPropertySet(int_t ifcModel, STRUCT__PROPERTY__SET ** propertySets, int_t ifcPropertySetInstance, STRUCT__SIUNIT * units)
{
	STRUCT__PROPERTY__SET	** ppPropertySet = propertySets;//&ifcObject->propertySets;
	while ((*ppPropertySet)) 
	{
		ppPropertySet = &(*ppPropertySet)->next;
	}

	wchar_t	* name = 0, *description = 0;
	sdaiGetAttrBN(ifcPropertySetInstance, (char*)L"Name", sdaiUNICODE, &name);
	sdaiGetAttrBN(ifcPropertySetInstance, (char*)L"Description", sdaiUNICODE, &description);

	_ASSERT((*ppPropertySet) == 0);
	(*ppPropertySet) = CreateIfcPropertySet(ifcPropertySetInstance, name, description);
	STRUCT__PROPERTY	** ppProperty = &(*ppPropertySet)->properties;

	int_t	* ifcPropertyInstances = 0;
#ifdef	_DEBUG
	//	int_t	ifcRelDefinesByType_TYPE = sdaiGetEntity(ifcModel, (char*) L"IFCRELDEFINESBYTYPE"),
	//			ifcRelDefinesByProperties_TYPE = sdaiGetEntity(ifcModel, (char*) L"IFCRELDEFINESBYPROPERTIES");
#endif

	sdaiGetAttrBN(ifcPropertySetInstance, (char*)L"HasProperties", sdaiAGGR, &ifcPropertyInstances);

	if (ifcPropertyInstances) {
		int_t	ifcPropertyInstancesCnt = sdaiGetMemberCount(ifcPropertyInstances);
		for (int_t i = 0; i < ifcPropertyInstancesCnt; ++i) {
			int_t	ifcPropertyInstance = 0,
				ifcPropertySingleValue_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCPROPERTYSINGLEVALUE");

			engiGetAggrElement(ifcPropertyInstances, i, sdaiINSTANCE, &ifcPropertyInstance);

			wchar_t	* propertyName = 0, *propertyDescription = 0;
			sdaiGetAttrBN(ifcPropertyInstance, (char*)L"Name", sdaiUNICODE, &propertyName);
			sdaiGetAttrBN(ifcPropertyInstance, (char*)L"Description", sdaiUNICODE, &propertyDescription);

			_ASSERT((*ppProperty) == 0);
			(*ppProperty) = CreateIfcProperty(ifcPropertyInstance, propertyName, propertyDescription);

			if (sdaiGetInstanceType(ifcPropertyInstance) == ifcPropertySingleValue_TYPE) {
				CreateIfcPropertySingleValue(ifcPropertyInstance, (*ppProperty), units);
			}
			else {
				//				ASSERT(false);
			}

			ppProperty = &(*ppProperty)->next;
		}
	}
}
void CreateIfcQuantityLength(int_t ifcQuantityLength, STRUCT__PROPERTY * ifcProperty, STRUCT__SIUNIT * units)
{
	wchar_t	*lengthValue = 0,
		    *unit = 0;
	int_t	ifcUnitInstance = 0;

	sdaiGetAttrBN(ifcQuantityLength, (char*)L"LengthValue", sdaiUNICODE, &lengthValue);
	sdaiGetAttrBN(ifcQuantityLength, (char*)L"Unit", sdaiUNICODE, &unit);
	sdaiGetAttrBN(ifcQuantityLength, (char*)L"Unit", sdaiINSTANCE, &ifcUnitInstance);

	_ASSERT(ifcUnitInstance == 0);

	ifcProperty->lengthValue = copyStr(lengthValue);
	ifcProperty->unit = copyStr(unit);
	if (unit == 0 || unit[0] == 0) {
		while (units) {
			if (units->type == LENGTHUNIT) {
				ifcProperty->unit = units->name;
			}
			units = units->next;
		}
	}
}
void CreateIfcQuantityArea(int_t ifcQuantityArea, STRUCT__PROPERTY * ifcProperty, STRUCT__SIUNIT * units)
{
	wchar_t	* areaValue = 0,
		*unit = 0;

	sdaiGetAttrBN(ifcQuantityArea, (char*)L"AreaValue", sdaiUNICODE, &areaValue);
	sdaiGetAttrBN(ifcQuantityArea, (char*)L"Unit", sdaiUNICODE, &unit);

	ifcProperty->areaValue = copyStr(areaValue);
	ifcProperty->unit = copyStr(unit);
	if (unit == 0 || unit[0] == 0) {
		while (units) {
			if (units->type == AREAUNIT) {
				ifcProperty->unit = units->name;
			}
			units = units->next;
		}
	}
}
void CreateIfcQuantityVolume(int_t ifcQuantityVolume, STRUCT__PROPERTY * ifcProperty, STRUCT__SIUNIT * units)
{
	wchar_t	* volumeValue = 0,
		*unit = 0;

	sdaiGetAttrBN(ifcQuantityVolume, (char*)L"VolumeValue", sdaiUNICODE, &volumeValue);
	sdaiGetAttrBN(ifcQuantityVolume, (char*)L"Unit", sdaiUNICODE, &unit);

	ifcProperty->volumeValue = copyStr(volumeValue);
	ifcProperty->unit = copyStr(unit);
	if (unit == 0 || unit[0] == 0) {
		while (units) {
			if (units->type == VOLUMEUNIT) {
				ifcProperty->unit = units->name;
			}
			units = units->next;
		}
	}
}
void CreateIfcQuantityCount(int_t ifcQuantityCount, STRUCT__PROPERTY * ifcProperty)//, STRUCT__SIUNIT * units)
{
	wchar_t	* countValue = 0,
		*unit = 0;

	sdaiGetAttrBN(ifcQuantityCount, (char*)L"CountValue", sdaiUNICODE, &countValue);
	sdaiGetAttrBN(ifcQuantityCount, (char*)L"Unit", sdaiUNICODE, &unit);

	ifcProperty->countValue = copyStr(countValue);
	ifcProperty->unit = copyStr(unit);
}
void CreateIfcQuantityWeigth(int_t ifcQuantityWeigth, STRUCT__PROPERTY * ifcProperty, STRUCT__SIUNIT * units)
{
	wchar_t	* weigthValue = 0,
		*unit = 0;

	sdaiGetAttrBN(ifcQuantityWeigth, (char*)L"WeigthValue", sdaiUNICODE, &weigthValue);
	sdaiGetAttrBN(ifcQuantityWeigth, (char*)L"Unit", sdaiUNICODE, &unit);

	ifcProperty->weigthValue = copyStr(weigthValue);
	ifcProperty->unit = copyStr(unit);
	if (unit == 0 || unit[0] == 0) {
		while (units) {
			if (units->type == MASSUNIT) {
				ifcProperty->unit = units->name;
			}
			units = units->next;
		}
	}
}
void CreateIfcQuantityTime(int_t ifcQuantityTime, STRUCT__PROPERTY * ifcProperty, STRUCT__SIUNIT * units)
{
	wchar_t	* timeValue = 0,
		*unit = 0;

	sdaiGetAttrBN(ifcQuantityTime, (char*)L"TimeValue", sdaiUNICODE, &timeValue);
	sdaiGetAttrBN(ifcQuantityTime, (char*)L"Unit", sdaiUNICODE, &unit);

	ifcProperty->timeValue = copyStr(timeValue);
	ifcProperty->unit = copyStr(unit);
	if (unit == 0 || unit[0] == 0) {
		while (units) {
			if (units->type == TIMEUNIT) {
				ifcProperty->unit = units->name;
			}
			units = units->next;
		}
	}
}
void CreateIfcElementQuantity(int_t ifcModel, STRUCT__PROPERTY__SET ** propertySets, int_t ifcPropertySetInstance, STRUCT__SIUNIT * units)
{
	STRUCT__PROPERTY__SET	** ppPropertySet = propertySets;//&ifcObject->propertySets;
	while ((*ppPropertySet)) {
		ppPropertySet = &(*ppPropertySet)->next;
	}

	wchar_t	* name = 0, *description = 0;
	sdaiGetAttrBN(ifcPropertySetInstance, (char*)L"Name", sdaiUNICODE, &name);
	sdaiGetAttrBN(ifcPropertySetInstance, (char*)L"Description", sdaiUNICODE, &description);

	_ASSERT((*ppPropertySet) == 0);
	(*ppPropertySet) = CreateIfcPropertySet(ifcPropertySetInstance, name, description);
	STRUCT__PROPERTY	** ppProperty = &(*ppPropertySet)->properties;

	int_t	* ifcQuantityInstances = 0;
#ifdef	_DEBUG
	//	int_t	ifcRelDefinesByType_TYPE = sdaiGetEntity(ifcModel, (char*) L"IFCRELDEFINESBYTYPE"),
	//			ifcRelDefinesByProperties_TYPE = sdaiGetEntity(ifcModel, (char*) L"IFCRELDEFINESBYPROPERTIES");
#endif

	sdaiGetAttrBN(ifcPropertySetInstance, (char*)L"Quantities", sdaiAGGR, &ifcQuantityInstances);

	if (ifcQuantityInstances) {
		int_t	ifcQuantityInstancesCnt = sdaiGetMemberCount(ifcQuantityInstances);
		for (int_t i = 0; i < ifcQuantityInstancesCnt; ++i) {
			int_t	ifcQuantityInstance = 0,
				ifcQuantityLength_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCQUANTITYLENGTH"),
				ifcQuantityArea_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCQUANTITYAREA"),
				ifcQuantityVolume_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCQUANTITYVOLUME"),
				ifcQuantityCount_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCQUANTITYCOUNT"),
				ifcQuantityWeigth_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCQUANTITYWEIGHT"),
				ifcQuantityTime_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCQUANTITYTIME");

			engiGetAggrElement(ifcQuantityInstances, i, sdaiINSTANCE, &ifcQuantityInstance);

			wchar_t	* quantityName = 0, *quantityDescription = 0;
			sdaiGetAttrBN(ifcQuantityInstance, (char*)L"Name", sdaiUNICODE, &quantityName);
			sdaiGetAttrBN(ifcQuantityInstance, (char*)L"Description", sdaiUNICODE, &quantityDescription);

			_ASSERT((*ppProperty) == 0);
			(*ppProperty) = CreateIfcProperty(ifcQuantityInstance, quantityName, quantityDescription);

			if (sdaiGetInstanceType(ifcQuantityInstance) == ifcQuantityLength_TYPE)
			{
				CreateIfcQuantityLength(ifcQuantityInstance, (*ppProperty), units);
			}
			else if (sdaiGetInstanceType(ifcQuantityInstance) == ifcQuantityArea_TYPE) {
				CreateIfcQuantityArea(ifcQuantityInstance, (*ppProperty), units);
			}
			else if (sdaiGetInstanceType(ifcQuantityInstance) == ifcQuantityVolume_TYPE) {
				CreateIfcQuantityVolume(ifcQuantityInstance, (*ppProperty), units);
			}
			else if (sdaiGetInstanceType(ifcQuantityInstance) == ifcQuantityCount_TYPE) {
				CreateIfcQuantityCount(ifcQuantityInstance, (*ppProperty));//, units);
			}
			else if (sdaiGetInstanceType(ifcQuantityInstance) == ifcQuantityWeigth_TYPE) {
				CreateIfcQuantityWeigth(ifcQuantityInstance, (*ppProperty), units);
			}
			else if (sdaiGetInstanceType(ifcQuantityInstance) == ifcQuantityTime_TYPE) {
				CreateIfcQuantityTime(ifcQuantityInstance, (*ppProperty), units);
			}
			else {
				_ASSERT(false);
			}

			ppProperty = &(*ppProperty)->next;
		}
	}
}
void CreateTypeObjectInstance(int_t ifcModel, STRUCT__PROPERTY__SET ** propertySets, int_t ifcTypeObjectInstance, STRUCT__SIUNIT * units)
{
	if (ifcTypeObjectInstance) 
	{
		int_t	* hasPropertySets = 0, hasPropertySetsCnt,
			    ifcElementQuantity_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCELEMENTQUANTITY"),
			    ifcPropertySet_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCPROPERTYSET");
		sdaiGetAttrBN(ifcTypeObjectInstance, (char*)L"HasPropertySets", sdaiAGGR, &hasPropertySets);
		hasPropertySetsCnt = sdaiGetMemberCount(hasPropertySets);
		for (int_t i = 0; i < hasPropertySetsCnt; ++i) 
		{
			int_t	hasPropertySetInstance = 0;
			engiGetAggrElement(hasPropertySets, i, sdaiINSTANCE, &hasPropertySetInstance);
			if (sdaiGetInstanceType(hasPropertySetInstance) == ifcElementQuantity_TYPE) 
			{
				CreateIfcElementQuantity(ifcModel, propertySets, hasPropertySetInstance, units);
			}
			else if (sdaiGetInstanceType(hasPropertySetInstance) == ifcPropertySet_TYPE) 
			{
				CreateIfcPropertySet(ifcModel, propertySets, hasPropertySetInstance, units);
			}
			else {
				_ASSERT(false);
			}
		}
	}
}
void CreateRelDefinesByProperties(int_t ifcModel, STRUCT__PROPERTY__SET ** propertySets, int_t ifcRelDefinesByProperties, STRUCT__SIUNIT * units)
{
	if (ifcRelDefinesByProperties) {
		int_t	ifcPropertySetInstance = 0,
			ifcElementQuantity_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCELEMENTQUANTITY"),
			ifcPropertySet_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCPROPERTYSET");

		sdaiGetAttrBN(ifcRelDefinesByProperties, (char*)L"RelatingPropertyDefinition", sdaiINSTANCE, &ifcPropertySetInstance);

		if (sdaiGetInstanceType(ifcPropertySetInstance) == ifcElementQuantity_TYPE) {
			CreateIfcElementQuantity(ifcModel, propertySets, ifcPropertySetInstance, units);
		}
		else if (sdaiGetInstanceType(ifcPropertySetInstance) == ifcPropertySet_TYPE) {
			CreateIfcPropertySet(ifcModel, propertySets, ifcPropertySetInstance, units);
		}
		else {
			//_ASSERT(false);
		}
	}
	else {
		_ASSERT(false);
	}
}
void CreateIfcInstanceProperties(int_t ifcModel, STRUCT__PROPERTY__SET ** propertySets, int_t ifcObjectInstance, STRUCT__SIUNIT * units)
{
	int_t		* isDefinedByInstances = 0,
		ifcRelDefinesByType_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCRELDEFINESBYTYPE"),
		ifcRelDefinesByProperties_TYPE = sdaiGetEntity(ifcModel, (char*)L"IFCRELDEFINESBYPROPERTIES");

	sdaiGetAttrBN(ifcObjectInstance, (char*)L"IsDefinedBy", sdaiAGGR, &isDefinedByInstances);

	if (isDefinedByInstances) {
		int_t	isDefinedByInstancesCnt = sdaiGetMemberCount(isDefinedByInstances);
		for (int_t i = 0; i < isDefinedByInstancesCnt; ++i) {
			int_t	isDefinedByInstance = 0;
			engiGetAggrElement(isDefinedByInstances, i, sdaiINSTANCE, &isDefinedByInstance);

			if (sdaiGetInstanceType(isDefinedByInstance) == ifcRelDefinesByType_TYPE) {
				int_t	typeObjectInstance = 0;

				sdaiGetAttrBN(isDefinedByInstance, (char*)L"RelatingType", sdaiINSTANCE, &typeObjectInstance);

				CreateTypeObjectInstance(ifcModel, propertySets, typeObjectInstance, units);
			}
			else if (sdaiGetInstanceType(isDefinedByInstance) == ifcRelDefinesByProperties_TYPE) {
				CreateRelDefinesByProperties(ifcModel, propertySets, isDefinedByInstance, units);
			}
			else {
				_ASSERT(false);
			}
		}
	}
}
