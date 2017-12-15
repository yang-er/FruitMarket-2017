#include "stdafx.h"
#include "structs.h"
#include "scan.h"
#include "math.h"
#include "stock.h"

const char* pfStock = "stock.dat";
stock warehouse[5];

/// <summary>从文件中加载库存信息</summary>
bool LoadStockFromFile()
{
	FILE *pFile;

	// 文件不存在则初始化默认值
	if (!file_exists(pfStock))
	{
		pFile = fopen(pfStock, "w");
		CheckFile(pFile, pfStock);
		fprintf(pFile, "苹果 公斤 F 20 0 5 0 0\n");
		fprintf(pFile, "香蕉 根 T 20 0 5 0 0\n");
		fprintf(pFile, "柚子 个 T 20 0 5 0 0\n");
		fprintf(pFile, "蓝莓 串 T 20 0 5 0 0\n");
		fprintf(pFile, "梨子 公斤 F 20 0 5 0 0\n");
		fclose(pFile);
	}

	// 读取文件内容
	pFile = fopen(pfStock, "r");
	CheckFile(pFile, pfStock);
	stock *pStock;
	char isSingle;
	int pFlag;
	for (int i = 0; i < 5; i++)
	{
		pStock = warehouse + i;
		pFlag = fscanf(pFile, "%s %s %c %d %d %d %d %d",
			pStock->fruitName, pStock->tagName, &isSingle,
			&pStock->left, &pStock->sold, &pStock->singlePrice,
			&pStock->todayUsage, &pStock->boxCount);
		if (pFlag != 8)
			DataNotFulfilled(pFile, pfStock);
		pStock->isSingled = (isSingle == 'T');
	}
	fclose(pFile);
	return true;
}

/// <summary>将水果库存信息保存到文件</summary>
bool SaveStockToFile()
{
	FILE *pFile;
	pFile = fopen(pfStock, "w");
	CheckFile(pFile, pfStock);
	stock *pStock;
	for (int i = 0; i < 5; i++)
	{
		pStock = warehouse + i;
		fprintf(pFile, "%s %s %c %d %d %d %d %d\n",
			pStock->fruitName, pStock->tagName, pStock->isSingled ? 'T' : 'F',
			pStock->left, pStock->sold, pStock->singlePrice,
			pStock->todayUsage, pStock->boxCount);
	}
	fclose(pFile);
	return true;
}

/// <summary>输出所有水果的库存</summary>
void OutputStock()
{
	printf("==================\n");
	printf("|    库存管理\n");
	printf("==================\n");
	for (int i = 0; i < 5; i++)
	{
		printf("|\n");
		printf("|  名称：%s\n", warehouse[i].fruitName);
		printf("|  单位：%s\n", warehouse[i].tagName);
		printf("|  单价：￥%.2lf\n", dollar(warehouse[i].singlePrice));
		if (warehouse[i].isSingled)
		{
			printf("|  一盒：%d个\n", warehouse[i].boxCount);
			printf("|  剩余：%d\n", warehouse[i].left);
			printf("|  卖出：%d\n", warehouse[i].sold);
		}
		else
		{
			printf("|  剩余：%.2lf\n", dollar(warehouse[i].left));
			printf("|  卖出：%.2lf\n", dollar(warehouse[i].sold));
		}
		printf("|  今销：￥%.2lf\n", dollar(warehouse[i].todayUsage));
		printf("|\n");
		printf("==================\n");
	}
}

/// <summary>增加某一水果的库存</summary>
bool AddStock()
{
	// 明确要操作的水果
	printf("目前的水果有：");
	for (int i = 0; i < 5; i++)
		printf("%d.%s ", i + 1, warehouse[i].fruitName);
	printf("\n");
	char op;
	op = ScanOption("请输入要进货的水果种类：", '1', '5');
	int id = op - '1';
	printf("当前水果的单位为：%s。\n", warehouse[id].tagName);

	// 获取进货数量
	if (warehouse[id].isSingled)
	{
		int count;
		ScanInt("请输入要增加的数量：", &count);
		warehouse[id].left += count;
		if (warehouse[id].left > 1000 || warehouse[id].left <= 0)
		{
			printf("爆仓了。自动将库存减到1000。\n");
			warehouse[id].left = 1000;
		}
	}
	else
	{
		double count;
		ScanDouble("请输入要增加的数量：", &count);
		warehouse[id].left += cent(count);
		if (warehouse[id].left > 10000 || warehouse[id].left <= 0)
		{
			printf("爆仓了。自动将库存减到100。\n");
			warehouse[id].left = 10000;
		}
	}
	return true;
}

/// <summary>修改某一水果的属性</summary>
bool ModifyStock()
{
	// 明确要操作的水果
	printf("目前的水果有：");
	for (int i = 0; i < 5; i++)
		printf("%d.%s ", i + 1, warehouse[i].fruitName);
	printf("\n");
	char op = ScanOption("请输入要修改的水果种类：", '1', '5');
	int id = op - '1';
	printf("==================\n");
	printf("|\n");
	printf("|  名称：%s\n", warehouse[id].fruitName);
	printf("|  单位：%s\n", warehouse[id].tagName);
	printf("|  单价：￥%.2lf\n", dollar(warehouse[id].singlePrice));
	if (warehouse[id].isSingled)
		printf("|  一盒：%d个\n", warehouse[id].sold);
	printf("|\n");
	printf("==================\n");
	printf("\n");

	// 获取进货数量
	if (ScanBoolean("是否修改名称？(y/n)："))
		ScanText("新名称：", warehouse[id].fruitName, 20);

	if (ScanBoolean("是否修改单位？(y/n)："))
		ScanText("单位名称：", warehouse[id].tagName, 20);
	
	if (ScanBoolean("是否修改单价？(y/n)："))
	{
		double p;
		ScanDouble("新的单价：", &p);
		warehouse[id].singlePrice = cent(p);
	}

	if (warehouse[id].isSingled && ScanBoolean("是否修改一盒的个数？(y/n)："))
		ScanInt("一盒个数：", &warehouse[id].boxCount);

	return true;
}

/// <summary>进入仓库模式</summary>
void _stock()
{
	clear();
	char op;
	while (true)
	{
		printf("==================\n");
		printf("|    水果库房\n");
		printf("==================\n");
		printf("|\n");
		printf("|    1.查库房\n");
		printf("|    2.进货\n");
		printf("|    3.修改\n");
		printf("|    4.退出\n");
		printf("|\n");
		printf("==================\n");
		op = ScanOption("请选择进入：", '1', '4');
		printf("\n");
		switch (op)
		{
		case '1':
			OutputStock();
			pause();
			break;
		case '2':
			AddStock();
			printf("添加完毕\n");
			sleep(500);
			break;
		case '4':
			if (ScanBoolean("确定退出嘛(y/n)："))
				op = -52;
			break;
		case '3':
			ModifyStock();
			printf("库存修改完毕\n");
			sleep(500);
			break;
		default:
			break;
		}

		// 退出时保存库存信息
		if (op == -52)
		{
			SaveStockToFile();
			sleep(500);
			clear();
			break;
		}

		// 清屏幕
		clear();
	}
}