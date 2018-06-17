#include "Kernel.h"
//#include"BlockDevice.h"
#include <stdio.h>
#include<iostream>

using namespace std;
myKernel myKernel::instance;

/*
* �豸�������ٻ������ȫ��manager
*/
myBufferManager g_BufferManager;

/*
* �ļ�ϵͳ���ȫ��manager
*/
myFileSystem g_FileSystem;
myFileManager g_FileManager;


//ע�����ο����е�user�ṹ
myUser g_u;


myKernel::myKernel()
{
}

myKernel::~myKernel()
{
}

myKernel& myKernel::Instance()
{
	return myKernel::instance;
}


void myKernel::InitBuffer(char*p)
{
	this->m_BufferManager = &g_BufferManager;


	printf("Initialize Buffer...");
	this->GetBufferManager().Initialize(p);
	printf("OK.\n");

}

void myKernel::InitFileSystem()
{
	this->m_FileSystem = &g_FileSystem;
	this->m_FileManager = &g_FileManager;
	this->m_u = &g_u;
	

	printf("Initialize File System...");
	this->GetFileSystem().Initialize();
	printf("OK.\n");

	printf("Initialize File Manager...");
	this->GetFileManager().Initialize();
	printf("OK.\n");
}

void myKernel::Initialize(char*p)
{
	InitBuffer(p);
	InitFileSystem();
}


myBufferManager& myKernel::GetBufferManager()
{
	return *(this->m_BufferManager);
}


myFileSystem& myKernel::GetFileSystem()
{
	return *(this->m_FileSystem);
}

myFileManager& myKernel::GetFileManager()
{
	return *(this->m_FileManager);
}

myUser& myKernel::GetUser()
{
	
	//m_u = &g_u;;

	return *(this->m_u);
}
