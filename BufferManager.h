#pragma once
#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "Buf.h"
//#include "DeviceManager.h"


//ע�����ο�����buffermangerֻ�账����mmap������ڴ�Ľ�����mmap������ڴ���൱�ڴ���
//ͬʱ1.����ֻ��һ���ļ�ϵͳ����������dev������豸����
//2.����Ҫ�ٸ������ͼ��Ľ���

//�����˺ܾã����ǵ�ʱ���ϵ�Ͳ����豸�����ˣ�
//buffermanager������д��ֱ�ӿ����Լ��Ļ��棬�������µ����豸����
class myBufferManager
{
public:
	/* static const member */
	static const int NBUF = 15;			/* ������ƿ顢������������ */
	static const int BUFFER_SIZE = 512;	/* ��������С�� ���ֽ�Ϊ��λ */

public:
	myBufferManager();
	~myBufferManager();

	void Initialize(char *start);					/* ������ƿ���еĳ�ʼ������������ƿ���b_addrָ����Ӧ�������׵�ַ��*/

	//ע����ʵ��ϵͳ����dev��ţ����ο�����û��

	myBuf* GetBlk(int blkno);				/* ����һ�黺�棬���ڶ�д�豸dev�ϵ��ַ���blkno��*/
	void Brelse(myBuf* bp);				/* �ͷŻ�����ƿ�buf */

	myBuf* Bread(int blkno);	/* ��һ�����̿顣devΪ�������豸�ţ�blknoΪĿ����̿��߼���š� */
	myBuf* Breada(short adev, int blkno, int rablkno);	/* ��һ�����̿飬����Ԥ����ʽ��
														* adevΪ�������豸�š�blknoΪĿ����̿��߼���ţ�ͬ����ʽ��blkno��
														* rablknoΪԤ�����̿��߼���ţ��첽��ʽ��rablkno�� */

	void Bwrite(myBuf* bp);				/* дһ�����̿� */
	void Bdwrite(myBuf* bp);				/* �ӳ�д���̿� */
	void Bawrite(myBuf* bp);				/* �첽д���̿� */

	void ClrBuf(myBuf* bp);				/* ��ջ��������� */
	void Bflush();						/* ��devָ���豸�������ӳ�д�Ļ���ȫ����������� */

	myBuf& GetBFreeList();				/* ��ȡ���ɻ�����п��ƿ�Buf�������� */

private:
	myBuf* InCore(int blkno);	/* ���ָ���ַ����Ƿ����ڻ����� */

private:
	myBuf bFreeList;						/* ���ɻ�����п��ƿ� */
	//һ��buf��Ӧһ���洢��
	myBuf m_Buf[NBUF];					/* ������ƿ����� */
	unsigned char Buffer[NBUF][BUFFER_SIZE];	/* ���������� */

	//���������ļ�ϵͳ��mmapӳ�䵽�ڴ�����ʼ��ַ
	char *p;
};

#endif
