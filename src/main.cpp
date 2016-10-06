#include <cstdio>
#include <algorithm>
#include <list>
#include <bitset>
#include <iostream>
using namespace std;

int data_num;//数据的个数

int data[1023][11];//储存数据，第一个index表示数据的位置，第二个index表示数据的10个特征字段（2个ip和2个端口号和协议号）

int buffer[33];//缓存每个特征字段每个特征值对应的bit字长(33X31 = 1024，和数据的data_num差不多)

struct Bitset{//储存每个特征字段每个特征字段每个特征值对应的bitset，按照要求用WAH方法存储。这个参考WAH压缩方法
	int list_num;
	int *bitlist;
};

struct Bitmap{//储存每个特征字段(共11个特征字段)的bitset信息
	int key_num;//每个特征字段包含的不重复的特征值个数
	int *key;//每个特征字段包含的不重复的特征值（从小到大排序）
	Bitset *bitset;//每个特征值的bitset
};

Bitmap bitmap_data[11];//定义11个特征字段的bitmap

void bitmap_processing(int index){//处理压缩的函数，index取值为0-10，表示11个不同的特征字段
	int ival[data_num];//暂存特征字段对应的特征值，方便后面排序
	for (int i = 0; i < data_num; ++i)
		ival[i] = data[i][index];//复制到暂存数组
	sort(ival,ival + data_num);//从小到大排序
	list<int> ival_list;//用链表来储存不同的特征值
	ival_list.push_back(ival[0]);//将最小的特征值存入链表
	int ival_num = 1;//记录不同特征值的个数，初始为1
	for (int i = 1; i < data_num; ++i){//遍历整个暂存数组，将所有不同的特征值存入链表中
		if(ival[i] != ival_list.back()){
			ival_list.push_back(ival[i]);
			ival_num++;
		}
	}
	bitmap_data[index].key_num = ival_num;//写入Bitmap中的key_num
	bitmap_data[index].key = new int[ival_num];//Bitmap中的key则是对应长度的数组，储存对应的特征值
	int j = 0;
	for (list<int>::iterator i = ival_list.begin(); i != ival_list.end(); ++i)
		bitmap_data[index].key[j++] = *i;


	bitmap_data[index].bitset = new Bitset[ival_num];//Bitmap中的bitset则是对应长度的数字，储存对应的位图信息

	for (int i = 0; i < bitmap_data[index].key_num; ++i){//对于每个特征值，都要建立一个索引位图
		for (j = 0; j < (data_num - 1)/31 + 1; ++j){//因为用int进行存储，每个是31位的
			buffer[j] = 0;
			for (int k = 0; k < 31 && j*31+k<data_num; ++k){//先储存在buffer数组中
				if (data[j*31 + k][index] == bitmap_data[index].key[i])
					buffer[j] += (1<<k);
			}
		}
		//下面进行压缩
		int ival_list_num = 1;//记录压缩后int的个数，初始为1
		list<int> ival_bitlist;//用链表来储存压缩后的bitset
		ival_bitlist.push_back(buffer[0]);
		if (buffer[0] != 0)
			ival_bitlist.back() += (1<<31);
		else
			ival_bitlist.back() ++;
		for (j = 1; j < data_num/31 + 1; ++j){
			if(buffer[j] == 0 && ival_bitlist.back() > 0)
				ival_bitlist.back()++;
			else{
				ival_bitlist.push_back(buffer[j]);
				ival_list_num++;
				if (buffer[j] != 0)
					ival_bitlist.back() += (1<<31);
				else
					ival_bitlist.back() ++;
			}
		}
		bitmap_data[index].bitset[i].list_num = ival_list_num;
		bitmap_data[index].bitset[i].bitlist = new int[ival_list_num];
		j = 0;
		for (list<int>::iterator k = ival_bitlist.begin(); k != ival_bitlist.end(); ++k)
			bitmap_data[index].bitset[i].bitlist[j++] = *k;
		//压缩完毕
	}
}

void io(){//从ipData.txt中读入数据，并且存入Data[][]中，里面的代码可以不用管
	FILE *fp=fopen("ipData.txt","r");//这里可以想想办法，我是不知道怎么用相对路径存储了，或者直接从网上down下来
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

void getresult(list<Bitset> bitsetlist,int bitsetnum){
	unsigned int result[(data_num - 1)/31 + 1];
	int num = (data_num - 1)/31 + 1;
	for (int i = 0; i < num; ++i)
		result[i] = 0xffffffff;
	for (int i = data_num - (num-1)*31; i < 31; ++i)
		result[num - 1] -= (1<<i);
		
	for (list<Bitset>::iterator b = bitsetlist.begin(); b != bitsetlist.end(); ++b){
		int ptr = 0;
		for (int ptrb = 0; ptrb < b->list_num; ++ptrb){
			if (b->bitlist[ptrb] > 0){
				for (int i = ptr; i < b->bitlist[ptrb] + ptr; ++i)
					result[i] = 0;
				ptr += b->bitlist[ptrb];
			}
			else {
				result[ptr] = result[ptr] & (unsigned int)b->bitlist[ptrb];
				ptr ++;
			}
		}
		// for (int i = 0; i < data_num/31 + 1; ++i){
		// 	bitset<32> a = b->bitlist[i];
		// 	cout<<a<<" ";
		// }
		// printf("\n");
	}

	list<int> resultlist;
	int resultnum = 0;
	for (int i = 0; i < data_num/31 + 1; ++i){
		if(result[i] != 0){
			for (int k = 0; k < 31; ++k){
				// bitset<32> a = result[i];
				// cout<<a<<" ";
				if (result[i] != 2 * (result[i]>>1)){
					resultnum++;
					resultlist.push_back(i*31 + k);
				}
				result[i] = result[i]>>1;
			}
			// bitset<32> a = result[i];
			// 	cout<<a<<endl;
		}
	}
	if (resultnum == 0){
		printf("查无对应数据\n");
		return;
	}
	printf("共查询到%d条数据，分别为：(编号按照输入顺序标号)\n", resultnum);
	int ival;
	for (list<int>::iterator b = resultlist.begin(); b != resultlist.end(); ++b){
		ival = *b;
		printf("编号：%d，  数据内容为：", ival + 1);
		for (int i = 0; i < 11; ++i)
			printf("%d ", data[ival][i]);
		printf("\n");
	}
	printf("\n\n\n");
	return;
}

int search(int* keylist,int num, int key){
	int lo = 0;int hi = num - 1;
	while(hi != lo){
		if (key <= keylist[(lo + hi)/2])
			hi = (lo + hi)/2;
		else
			lo = (lo + hi)/2 + 1;	
	}
	if (key == keylist[hi])
		return hi;
	else return -1;
}

void query(){
	printf("已经完成数据库的建立.\n");
	while(1){
		printf("请输入查询信息：\n(查询输入以q开头，空一格，之后输入11个数字，以空格隔开，表示11个字段索要查找的数值，-1表示该字段忽略；退出请输入t)\n");
		char ivalchar;
		scanf("%c",&ivalchar);
		if (ivalchar == 't')
			return;
		int qkey[11];
		for (int i = 0; i < 11; ++i)
			scanf(" %d",&qkey[i]);
		getchar();
		list<Bitset> bitsetlist;
		int bitsetnum = 0;
		int if_exist = 1;
		for (int i = 0; i < 11; ++i){
			if(qkey[i]!=-1){
				int *ivalkeylist = bitmap_data[i].key;
				int ivalkey_num = bitmap_data[i].key_num;
				int qbitsetindex = search(ivalkeylist,ivalkey_num,qkey[i]);
				if (qbitsetindex == -1){
					if_exist = 0;
					break;	
				}
				bitsetlist.push_back(bitmap_data[i].bitset[qbitsetindex]);
				bitsetnum++;
			}
		}
		if(if_exist == 0){
			printf("查无对应数据！\n\n");
			continue;
		}
		printf("开始查询...\n");
		printf("%d\n", bitsetnum);
		getresult(bitsetlist,bitsetnum);
	}
}

int main(int argc, char const *argv[])
{
	printf("正在建立数据库...\n");
	io();
	for (int i = 0; i < 11; ++i)
		bitmap_processing(i);
	query();
	printf("欢迎再次使用，再见！\n");
	return 0;
}
