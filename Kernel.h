#pragma once
#ifndef KERNEL_H
#define KERNEL_H

#include "user.h"
#include "BufferManager.h"
#include "openfilemanager.h"
#include "filesystem.h"

/*
* Kernel�����ڷ�װ�����ں���ص�ȫ����ʵ������
* ����PageManager, ProcessManager�ȡ�
*
* Kernel�����ڴ���Ϊ����ģʽ����֤�ں��з�װ���ں�
* ģ��Ķ���ֻ��һ��������
*/
class myKernel
{

public:
	myKernel();
	~myKernel();
	static myKernel& Instance();
	void Initialize(char*p);		/* �ú�����ɳ�ʼ���ں˴󲿷����ݽṹ�ĳ�ʼ�� */

	myBufferManager& GetBufferManager();
	myFileSystem& GetFileSystem();
	myFileManager& GetFileManager();
	myUser& GetUser();		/* ��ȡ��ǰ���̵�User�ṹ */

private:
	void InitBuffer(char*p);
	void InitFileSystem();

private:
	static myKernel instance;		/* Kernel������ʵ�� */

	myBufferManager* m_BufferManager;
	myFileSystem* m_FileSystem;
	myFileManager* m_FileManager;
	myUser *m_u;

};



#endif
