#include "stdafx.h"
#include "FbxParser.h"
#include "Include.h"


void CFbxParser::ParsingVertex(FbxNode* _pNode, Animation* _pAnimation)
{
	map<unsigned int, vector<unsigned int>> mVertexByIndex;
	FbxNodeAttribute *pFbxNodeAttribute = _pNode->GetNodeAttribute();

	_pAnimation->pAniBuffer->m_strName = _pNode->GetName();

	if (pFbxNodeAttribute != nullptr && pFbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		FbxMesh* pMesh = _pNode->GetMesh();

		//���ý� ī��Ʈ �ε�
		const unsigned int VertexCount = pMesh->GetControlPointsCount();
		const unsigned int IndexCount = pMesh->GetPolygonVertexCount();

		//��������
		FbxVector4* IVertexArray = new FbxVector4[VertexCount];
		for (unsigned int i = 0; i < VertexCount; i++)
			IVertexArray[i] = pMesh->GetControlPoints()[i];


		int* IndexArray = new int[IndexCount];
		for (unsigned int i = 0; i < IndexCount; i++)
			IndexArray[i] = pMesh->GetPolygonVertices()[i];

		_pAnimation->pAniBuffer->SetVertexSize(IndexCount);
		for (unsigned int i = 0; i < IndexCount; i++)
		{
			//���ؽ��� ������ġ ������ ������
			_pAnimation->pAniBuffer->GetVertex(i)->SetPos(
				(float)IVertexArray[IndexArray[i]].mData[0],
				(float)IVertexArray[IndexArray[i]].mData[1],
				(float)IVertexArray[IndexArray[i]].mData[2]);

			mVertexByIndex[IndexArray[i]].push_back(i);
		}

		//�븻����
		FbxArray<FbxVector4> NormalArray;

		pMesh->GetPolygonVertexNormals(NormalArray);

		for (unsigned int i = 0; i < IndexCount; i++)
		{
			//���ؽ��� �븻��ġ ������ ������
			_pAnimation->pAniBuffer->GetVertex(i)->SetNormal(
				(float)NormalArray[i].mData[0],
				(float)NormalArray[i].mData[1],
				(float)NormalArray[i].mData[2]);
		}

		//uv����
		FbxArray<FbxVector2> UVArray;
		FbxStringList uvNameList;
		pMesh->GetUVSetNames(uvNameList);
		pMesh->GetPolygonVertexUVs(uvNameList.GetStringAt(0), UVArray);
		for (unsigned int i = 0; i < IndexCount; ++i)
		{
			//uv�迭���� i��° �븻�� ������ �ش� ������ �Ž��� i��° ������ �����Ѵ�.
			_pAnimation->pAniBuffer->GetVertex(i)->vTextureUV.x = (float)UVArray[i].mData[0];
			_pAnimation->pAniBuffer->GetVertex(i)->vTextureUV.y = 1.0f - (float)UVArray[i].mData[1];
		}

		FbxGeometry* pGeo = _pNode->GetGeometry();
		int SkinCount = pGeo->GetDeformerCount(FbxDeformer::eSkin);//��Ų�� ����

		for (int i = 0; i < SkinCount; i++)
		{
			FbxSkin* pSkin = (FbxSkin*)pGeo->GetDeformer(i, FbxDeformer::eSkin);
			int ClusterCount = pSkin->GetClusterCount();//Ŭ�������� ����

			for (int j = 0; j < ClusterCount; j++)
			{
				FbxCluster* pCluster = pSkin->GetCluster(j);
				int ClusterIndexCount = pCluster->GetControlPointIndicesCount();//Ŭ������ �ε����� ����
				int* ClusterIndices = pCluster->GetControlPointIndices();
				double* ClusterWeights = pCluster->GetControlPointWeights();

				for (int k = 0; k < ClusterIndexCount; k++)
				{
					string BoneName = string(pCluster->GetLink()->GetName());

					int Index = _pAnimation->mapIndexByName[BoneName];

					FbxAMatrix LinkBoneMtx;//���� ����Ʈ����
					FbxAMatrix TransBoneMtx;//�̵� ����Ʈ����
					FbxAMatrix ResultMatrix;//���� ����Ʈ����

					pCluster->GetTransformLinkMatrix(LinkBoneMtx);
					pCluster->GetTransformMatrix(TransBoneMtx);


					ResultMatrix = LinkBoneMtx.Inverse() *  TransBoneMtx; //���� ����Ʈ������ ������ ����� �̸� �̵� ���� ���Ѵ�.

					for (int m = 0; m < 4; ++m)
						for (int n = 0; n < 4; ++n)
							_pAnimation->pBaseBoneMatrix[Index].m[m][n] = (float)(ResultMatrix.Get(m, n));


					float BoneWeight = (float)ClusterWeights[k];
					int BoneIndex = ClusterIndices[k];

					for (auto iter : mVertexByIndex[BoneIndex])
					{

						if (Index == 0 || Index == -1)
						{
							//�ε����� ����.
						}
						else
						{
							_pAnimation->pAniBuffer->m_pVertex[iter].AddBone(Index, BoneWeight);
						}
					}
				}
			}

		}

	}
	else
	{
		//�ش�Ž��� ��Ʈ����Ʈ�� ����.
	}

	_pAnimation->pAniBuffer->CreateBuffer();

	unsigned int nNodeChild = _pNode->GetChildCount();

	for (unsigned int i = 0; i < nNodeChild; i++)
	{
		FbxNode *pChildNode = _pNode->GetChild(i);

		CAniBuffer* pChild = new CAniBuffer;				//�߰��� �ڽ� �Ҵ�
		_pAnimation->pAniBuffer->AddChild(pChild);			//�ڽ� �߰�

		ParsingVertex(pChildNode, _pAnimation);
	}

}

void CFbxParser::ParsingAnimation(FbxNode* _pNode,
	Animation* _pAnimation,
	FbxAnimStack* _pAniStatck)
{
	//std::cout<<pNode->GetName()<<"\'s Child Count :"<<pNode->GetChildCount()<<std::endl;

	if (_pNode != nullptr)
	{
		unsigned int BoneIndex = _pAnimation->mapIndexByName[_pNode->GetName()];	//���� �ε����� �����´�.

																					//std::cout << _pNode->GetName() << "(INDEX:" << g_mIndexByName[_pNode->GetName()] << ") Load..." << g_mIndexByName.size() << std::endl;

		{
			FbxTime maxTime = _pAniStatck->GetLocalTimeSpan().GetDuration();
			for (long long i = 0; i < maxTime.GetMilliSeconds() / 10; i++)
			{
				FbxTime n_time;
				n_time.SetMilliSeconds(i * 10);

				for (int m = 0; m < 4; ++m)
					for (int n = 0; n < 4; ++n)
						_pAnimation->ppAniMatrix[i][BoneIndex].m[m][n] = (float)((_pNode->EvaluateGlobalTransform(n_time)).Get(m, n));

			}
		}
	}

	for (int i = 0; i < _pNode->GetChildCount(); ++i)
		ParsingAnimation(_pNode->GetChild(i), _pAnimation, _pAniStatck);
}