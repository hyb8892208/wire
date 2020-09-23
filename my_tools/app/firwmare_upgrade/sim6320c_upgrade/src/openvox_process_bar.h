#ifndef UPGRADE_COUNT_H

#define UPGRADE_COUNT_H

#include <stdio.h> //for FILE*
class UpgradeCount{
private:
	static UpgradeCount *instance;
	long long m_total_size;//��Ҫд����ܴ�С
	long long m_write_size;//�Ѿ�д����ļ���С
	int m_last_size;//���һ��д�뵽�������ļ��еĴ�С
	FILE *m_handle;//���������
	UpgradeCount();
public:
	static UpgradeCount *getInstance();//��ȡ����
	~UpgradeCount();
	void set_total_size(long long total_size);//�����ܴ�С
	void set_write_size(long long write_size);//�����Ѿ�д����ļ���С
	void process_bar(char *result);//�������ӿ�
};

#endif
