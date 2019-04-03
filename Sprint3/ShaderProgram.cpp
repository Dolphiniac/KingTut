#include "ShaderProgram.h"
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

ShaderProgram * ShaderProgram::Create( const char * shaderName ) {
	ShaderProgram * result = new ShaderProgram;

	std::string filename = shaderName;
	filename.append( ".vspv" );

	HANDLE fileHandle = CreateFile( filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	DWORD fileSize = GetFileSize( fileHandle, NULL );

	char * spirvBuffer = new char[ fileSize ];
	DWORD bytesRead;
	ReadFile( fileHandle, spirvBuffer, fileSize, &bytesRead, NULL );

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = fileSize;
	shaderModuleCreateInfo.pCode = ( uint32_t * )spirvBuffer;
	VK_CHECK( vkCreateShaderModule( renderObjects.device, &shaderModuleCreateInfo, NULL, &result->m_vertexShader ) );

	delete[] spirvBuffer;
	CloseHandle( fileHandle );
	
	filename = shaderName;
	filename.append( ".fspv" );

	fileHandle = CreateFile( filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	fileSize = GetFileSize( fileHandle, NULL );

	spirvBuffer = new char[ fileSize ];
	ReadFile( fileHandle, spirvBuffer, fileSize, &bytesRead, NULL );

	shaderModuleCreateInfo.codeSize = fileSize;
	shaderModuleCreateInfo.pCode = ( uint32_t * )spirvBuffer;
	VK_CHECK( vkCreateShaderModule( renderObjects.device, &shaderModuleCreateInfo, NULL, &result->m_fragmentShader ) );

	delete[] spirvBuffer;
	CloseHandle( fileHandle );

	return result;
}