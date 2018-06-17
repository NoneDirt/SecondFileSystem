#pragma once

#include"file.h"
#include"filesystem.h"
#include"inode.h"

/* Forward Declaration */
class myOpenFileTable;
class myInodeTable;
class mySuperBlock;
class myFileSystem;

/* ����2������ʵ��������OpenFileManager.cpp�ļ��� */
extern myInodeTable g_InodeTable;
extern myOpenFileTable g_OpenFileTable;
extern mySuperBlock g_spb;
/*
* ���ļ�������(OpenFileManager)����
* �ں��жԴ��ļ������Ĺ���Ϊ����
* ���ļ������ں����ݽṹ֮��Ĺ���
* ��ϵ��
* ������ϵָ����u���д��ļ�������ָ��
* ���ļ����е�File���ļ����ƽṹ��
* �Լ���File�ṹָ���ļ���Ӧ���ڴ�Inode��
*/
class myOpenFileTable
{
public:
	/* static consts */
	//static const int NINODE	= 100;	/* �ڴ�Inode������ */
	static const int NFILE = 100;	/* ���ļ����ƿ�File�ṹ������ */

									/* Functions */
public:
	/* Constructors */
	myOpenFileTable();
	/* Destructors */
	~myOpenFileTable();

	// /* 
	// * @comment �����û�ϵͳ�����ṩ���ļ�����������fd��
	// * �ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ
	// */
	// File* GetF(int fd);
	/*
	* @comment ��ϵͳ���ļ����з���һ�����е�File�ṹ
	*/
	myFile* FAlloc();
	/*
	* @comment �Դ��ļ����ƿ�File�ṹ�����ü���f_count��1��
	* �����ü���f_countΪ0�����ͷ�File�ṹ��
	*/
	void CloseF(myFile* pFile);

	/* Members */
public:
	myFile m_File[NFILE];			/* ϵͳ���ļ���Ϊ���н��̹������̴��ļ���������
								�а���ָ����ļ����ж�ӦFile�ṹ��ָ�롣*/
};

/*
* �ڴ�Inode��(class myInodeTable)
* �����ڴ�Inode�ķ�����ͷš�
*/
class myInodeTable
{
	/* static consts */
public:
	static const int NINODE = 100;	/* �ڴ�Inode������ */

									/* Functions */
public:
	/* Constructors */
	myInodeTable();
	/* Destructors */
	~myInodeTable();

	/*
	* @comment ��ʼ����g_FileSystem���������
	*/
	void Initialize();
	/*
	* @comment ����ָ���豸��dev�����Inode��Ż�ȡ��Ӧ
	* myInode�������Inode�Ѿ����ڴ��У��������������ظ��ڴ�Inode��
	* ��������ڴ��У���������ڴ�����������ظ��ڴ�Inode
	*/
	myInode* IGet( int inumber);
	/*
	* @comment ���ٸ��ڴ�Inode�����ü����������Inode�Ѿ�û��Ŀ¼��ָ������
	* ���޽������ø�Inode�����ͷŴ��ļ�ռ�õĴ��̿顣
	*/
	void IPut(myInode* pNode);

	/*
	* @comment �����б��޸Ĺ����ڴ�Inode���µ���Ӧ���Inode��
	*/
	void UpdateInodeTable();

	/*
	* @comment ����豸dev�ϱ��Ϊinumber�����inode�Ƿ����ڴ濽����
	* ������򷵻ظ��ڴ�Inode���ڴ�Inode���е�����
	*/
	int IsLoaded( int inumber);
	/*
	* @comment ���ڴ�Inode����Ѱ��һ�����е��ڴ�Inode
	*/
	myInode* GetFreeInode();

	/* Members */
public:
	myInode m_Inode[NINODE];		/* �ڴ�Inode���飬ÿ�����ļ�����ռ��һ���ڴ�Inode */

	myFileSystem* m_FileSystem;	/* ��ȫ�ֶ���g_FileSystem������ */
};

