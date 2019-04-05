/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"


namespace ve {

	//---------------------------------------------------------------------
	//Mesh

	/**
	*
	* \brief VEMesh constructor from an Assimp aiMesh.
	*
	* Create a VEMesh from an Assmip aiMesh input.
	*
	* \param[in] name The name of the mesh.
	* \param[in] paiMesh Pointer to the Assimp aiMesh that is the source of this mesh.
	*
	*/

	VEMesh::VEMesh(std::string name, const aiMesh *paiMesh) : VENamedClass(name) {
		std::vector<vh::vhVertex>	vertices;	//vertex array
		std::vector<uint32_t>		indices;	//index array

												//copy the mesh vertex data
		m_vertexCount = paiMesh->mNumVertices;
		float maxsq = 0;
		for (uint32_t i = 0; i < paiMesh->mNumVertices; i++) {
			vh::vhVertex vertex;
			vertex.pos.x = paiMesh->mVertices[i].x;								//copy 3D position in local space
			vertex.pos.y = paiMesh->mVertices[i].y;
			vertex.pos.z = paiMesh->mVertices[i].z;

			maxsq = std::max(std::max(vertex.pos.x*vertex.pos.x, vertex.pos.y*vertex.pos.y), vertex.pos.z*vertex.pos.z);

			if (paiMesh->HasNormals()) {										//copy normals
				vertex.normal.x = paiMesh->mNormals[i].x;
				vertex.normal.y = paiMesh->mNormals[i].y;
				vertex.normal.z = paiMesh->mNormals[i].z;
			}

			if (paiMesh->HasTangentsAndBitangents() && paiMesh->mTangents) {	//copy tangents
				vertex.tangent.x = paiMesh->mTangents[i].x;
				vertex.tangent.y = paiMesh->mTangents[i].y;
				vertex.tangent.z = paiMesh->mTangents[i].z;
			}

			if (paiMesh->HasTextureCoords(0)) {									//copy texture coordinates
				vertex.texCoord.x = paiMesh->mTextureCoords[0][i].x;
				vertex.texCoord.y = paiMesh->mTextureCoords[0][i].y;
			}

			vertices.push_back(vertex);
		}
		m_boundingSphereRadius = sqrt(maxsq);

		//got through the aiMesh faces, and copy the indices
		m_indexCount = 0;
		for (uint32_t i = 0; i < paiMesh->mNumFaces; i++) {
			for (uint32_t j = 0; j < paiMesh->mFaces[i].mNumIndices; j++) {
				indices.push_back(paiMesh->mFaces[i].mIndices[j]);
				m_indexCount++;
			}
		}

		//create the vertex buffer
		vh::vhBufCreateVertexBuffer(getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
			getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
			vertices, &m_vertexBuffer, &m_vertexBufferAllocation);

		//create the index buffer
		vh::vhBufCreateIndexBuffer(getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
			getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
			indices, &m_indexBuffer, &m_indexBufferAllocation);
	}

	/**
	* \brief Destroy the vertex and index buffers
	*/
	VEMesh::~VEMesh() {
		vmaDestroyBuffer(getRendererPointer()->getVmaAllocator(), m_indexBuffer, m_indexBufferAllocation);
		vmaDestroyBuffer(getRendererPointer()->getVmaAllocator(), m_vertexBuffer, m_vertexBufferAllocation);
	}


	//---------------------------------------------------------------------
	//Material

	/**
	* \brief Destroy the material textures
	*/
	VEMaterial::~VEMaterial() {
		if (mapDiffuse != nullptr) delete mapDiffuse;
		if (mapBump != nullptr) delete mapBump;
		if (mapNormal != nullptr) delete mapNormal;
		if (mapHeight != nullptr) delete mapHeight;
	};


	//---------------------------------------------------------------------
	//Texture

	/**
	*
	* \brief VETexture constructor from a list of textures.
	*
	* Create a VETexture from a list of textures. The textures must lie in the same directory and are stored in a texture array.
	* This can be used also as a cube map.
	*
	* \param[in] name The name of the mesh.
	* \param[in] basedir Name of the directory the files are in.
	* \param[in] texNames List of filenames of the textures.
	* \param[in] flags Vulkan flags for creating the textures.
	* \param[in] viewType Vulkan view tape for the image view.
	*
	*/
	VETexture::VETexture(std::string name, std::string &basedir, std::vector<std::string> texNames,
		VkImageCreateFlags flags, VkImageViewType viewType) : VENamedClass(name) {
		if (texNames.size() == 0) return;

		vh::vhBufCreateTextureImage(getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
			getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
			basedir, texNames, flags, &m_image, &m_deviceAllocation, &m_extent);

		m_format = VK_FORMAT_R8G8B8A8_UNORM;
		vh::vhBufCreateImageView(getRendererPointer()->getDevice(), m_image,
			m_format, viewType, (uint32_t)texNames.size(), VK_IMAGE_ASPECT_COLOR_BIT, &m_imageView);

		vh::vhBufCreateTextureSampler(getRendererPointer()->getDevice(), &m_sampler);
	}

	/**
	*
	* \brief VETexture constructor from a GLI cube map file.
	*
	* Create a VETexture from a GLI cubemap file. This has been loaded using GLI from a ktx or dds file.
	*
	* \param[in] name The name of the mesh.
	* \param[in] texCube The GLI cubemap structure
	* \param[in] flags Create flags for the images (e.g. Cube compatible or array)
	* \param[in] viewType Type for the image views
	*
	*/
	VETexture::VETexture(std::string name, gli::texture_cube &texCube,
		VkImageCreateFlags flags, VkImageViewType viewType) : VENamedClass(name) {

		vh::vhBufCreateTexturecubeImage(getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
			getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
			texCube, &m_image, &m_deviceAllocation, &m_format);

		m_extent.width = texCube.extent().x;
		m_extent.height = texCube.extent().y;

		vh::vhBufCreateImageView(getRendererPointer()->getDevice(), m_image,
			m_format, VK_IMAGE_VIEW_TYPE_CUBE, 6,
			VK_IMAGE_ASPECT_COLOR_BIT, &m_imageView);

		vh::vhBufCreateTextureSampler(getRendererPointer()->getDevice(), &m_sampler);
	}

	/**
	* \brief VETexture destructor - destroy the sampler, image view and image
	*/
	VETexture::~VETexture() {
		if (m_sampler != VK_NULL_HANDLE) vkDestroySampler(getRendererPointer()->getDevice(), m_sampler, nullptr);
		if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(getRendererPointer()->getDevice(), m_imageView, nullptr);
		if (m_image != VK_NULL_HANDLE) vmaDestroyImage(getRendererPointer()->getVmaAllocator(), m_image, m_deviceAllocation);
	}


}


