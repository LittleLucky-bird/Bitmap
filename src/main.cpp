#include <cstdio>
#include <algorithm>
#include <list>
using namespace std;

unsigned int data_num;//数据的个数

unsigned int data[1024][11];//储存数据，第一个index表示数据的位置，第二个index表示数据的10个特征字段（2个ip和2个端口号和协议号）

unsigned int buffer[32];//缓存每个特征字段每个特征值对应的bit字长(32X32 = 1024，和数据的data_num差不多)

struct Bitset{//储存每个特征字段每个特征字段每个特征值对应的bitset，按照要求用WAH方法存储。这个参考WAH压缩方法
	unsigned int list_num;
	unsigned int *bitlist;
	unsigned int *mapnum;
};

struct Bitmap{//储存每个特征字段(共11个特征字段)的bitset信息
	unsigned int key_num;//每个特征字段包含的不重复的特征值个数
	unsigned int *key;//每个特征字段包含的不重复的特征值（从小到大排序）
	Bitset *bitset;//每个特征值的bitset
};

Bitmap bitmap_data[11];//定义11个特征字段的bitmap

void bitmap_processing(int index){//处理压缩的函数，index取值为0-10，表示11个不同的特征字段
	int ival[data_num];//暂存特征字段对应的特征值，方便后面排序
	for (int i = 0; i < data_num; ++i)
		ival[i] = data[i][index];//复制到暂存数组
	sort(ival,ival + data_num);//从小到大排序
	list<unsigned int> ival_list;//用链表来储存不同的特征值
	ival_list.push_back(ival[0]);//将最小的特征值存入链表
	int ival_num = 1;//记录不同特征值的个数，初始为1
	for (int i = 1; i < data_num; ++i){//遍历整个暂存数组，将所有不同的特征值存入链表中
		if(ival[i] != ival_list.back()){
			ival_list.push_back(ival[i]);
			ival_num++;
		}
	}
	bitmap_data[index].key_num = ival_num;//写入Bitmap中的key_num
	bitmap_data[index].key = new unsigned int[ival_num];//Bitmap中的key则是对应长度的数组，储存对应的特征值
	int j = 0;
	for (list<unsigned int>::iterator i = ival_list.begin(); i != ival_list.end(); ++i)
		bitmap_data[index].key[j++] = *i;


	bitmap_data[index].bitset = new Bitset[ival_num];//Bitmap中的bitset则是对应长度的数字，储存对应的位图信息

	for (int i = 0; i < bitmap_data[index].key_num; ++i){//对于每个特征值，都要建立一个索引位图
		for (j = 0; j < data_num/32 + 1; ++j){//因为用unsigned int进行存储，每个是32位的
			buffer[j] = 0;
			for (int k = 0; k < 32 && j*32+k<data_num; ++k){//先储存在buffer数组中
				if (data[j*32 + k][index] == bitmap_data[index].key[i])
					buffer[j] += (1<<k);
			}
		}
		//下面进行压缩
		int ival_list_num = 1;//记录压缩后unsigned int的个数，初始为1
		list<unsigned int> ival_bitlist;//用链表来储存压缩后的bitset
		list<unsigned int> ivallistnum;//用链表来储存压缩后各bitset对应的数量
		ival_bitlist.push_back(buffer[0]);
		ivallistnum.push_back(1);
		for (j = 1; j < data_num/32 + 1; ++j){
			if(buffer[j] == ival_bitlist.back())
				ivallistnum.back()++;
			else{
				ival_bitlist.push_back(buffer[j]);
				ivallistnum.push_back(1);
				ival_list_num++;
			}
		}
		bitmap_data[index].bitset[i].list_num = ival_list_num;
		bitmap_data[index].bitset[i].bitlist = new unsigned int[ival_list_num];
		bitmap_data[index].bitset[i].mapnum = new unsigned int[ival_list_num];
		j = 0;
		for (list<unsigned int>::iterator k = ival_bitlist.begin(); k != ival_bitlist.end(); ++k)
			bitmap_data[index].bitset[i].bitlist[j++] = *k;
		j = 0;
		for (list<unsigned int>::iterator k = ivallistnum.begin(); k != ivallistnum.end(); ++k)
			bitmap_data[index].bitset[i].mapnum[j++] = *k;
		//压缩完毕
	}
}

void io(){//从ipData.txt中读入数据，并且存入Data[][]中，里面的代码可以不用管
	FILE *fp=fopen("Desktop/Bitmap/src/ipData.txt","r");//这里可以想想办法，我是不知道怎么用相对路径存储了，或者直接从网上down下来
	int i = 0;
	while(!feof(fp)){
		for (int j = 0; j < 11; ++j){
			fscanf(fp,"%d",&data[i][j]);
			fgetc(fp);
		}
		i++;
	}
	data_num = i - 1;
	fclose(fp);
}


int main(int argc, char const *argv[])
{
	printf("正在建立数据库...\n");
	io();
	for (int i = 0; i < 11; ++i)
		bitmap_processing(i);

	//这一部分用来检验的
	// int k = 9;
	// for (int i = 0; i < bitmap_data[k].key_num; ++i){
	// 	printf("\n\n\n\n         %d        \n\n\n\n\n", bitmap_data[k].key[i]);
	// 	Bitset b = bitmap_data[k].bitset[i];
	// 	for (int j = 0; j < b.list_num; ++j){
	// 		printf("%u      %d\n",b.bitlist[j],b.mapnum[j]);
	// 	}
	// 	printf("\n");
	// }
	//这一部分用来检验的

	printf("已经完成数据库的建立，请输入查询信息：\n（查询输入以q开头，空一格，之后输入11个数字，以空格隔开，表示11个字段索要查找的数值，-1表示该字段忽略；退出请输入t）\n");

	return 0;
}
