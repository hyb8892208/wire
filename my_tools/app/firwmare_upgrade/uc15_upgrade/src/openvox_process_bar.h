#ifndef UPGRADE_COUNT_H

#define UPGRADE_COUNT_H

#include <stdio.h> //for FILE*
class UpgradeCount{
private:
	static UpgradeCount *instance;
	long long m_total_size;//需要写入的总大小
	long long m_write_size;//已经写入的文件大小
	int m_last_size;//最后一次写入到进度条文件中的大小
	FILE *m_handle;//进度条句柄
	UpgradeCount();
public:
	static UpgradeCount *getInstance();//获取对象
	~UpgradeCount();
	void set_total_size(long long total_size);//设置总大小
	void set_write_size(long long write_size);//设置已经写入的文件大小
	void process_bar(char *result);//进度条接口
};

#endif
