#include "stdafx.h"
#include "structs.h"
#include "scan.h"
#include "ticket.h"
#include "user.h"
#include "stock.h"

ticket *pTicketFront = NULL;
ticket *pTicketRear = NULL;
ticket *pTicketTemp = NULL;
const char *pfTicket = "ticket.dat";

ticket *FindTicket(short tid)
{
	ticket* temp;
	temp = pTicketFront;
	while (temp != NULL && temp->tid != tid)
		temp = temp->next;
	return temp;
}

void LoadTicketFromFile()
{
	return;
}

bool SaveTicketToFile()
{
	return false;
}

// 释放并结束
#define doClean(bContinue, b) do { \
			_free(pTicketTemp, ticket); \
			pTicketTemp = NULL; \
			bContinue = b; \
		} while (false)

bool AddTicket()
{
	do {
		_alloc(pTicketTemp, ticket);

		// 确定小票编号
		bool bContinue = true;
		do {
			ScanShort("请输入序号：", &pTicketTemp->tid, false);
			bContinue = FindTicket(pTicketTemp->tid) != NULL;
			if (bContinue) printf("小票%04d已存在！\n", pTicketTemp->tid);
			if (bContinue && !ScanBoolean("是否更换号码？(y/n)：")) doClean(bContinue, false);
		} while (bContinue);
		if (pTicketTemp == NULL) continue;

		// 确定购买时间
		pTicketTemp->time = ScanTime("请输入下单时间：");

		// 确定购买数量和总金额
		printf("以下不够买请输入0。\n");
		double c = 0;
		int d = 0, sum = 0;
		char msg[50];
		for (int i = 0; i < 5; i++)
		{
			sprintf(msg, "请输入%s购买数量（%.2lf元/%s）：", warehouse[i].fruitName, dollar(warehouse[i].singlePrice), warehouse[i].tagName);
			if (warehouse[i].isSingled)
			{
				do { ScanInt(msg, &d); } while (d < 0 && printf("购买数量不合法！\n"));
				pTicketTemp->amount[i] = d;
				pTicketTemp->credit[i] = d * warehouse[i].singlePrice;
			}
			else
			{
				do { ScanDouble(msg, &c); } while (c < 0 && printf("购买数量不合法！\n"));
				pTicketTemp->amount[i] = cent(c);
				pTicketTemp->credit[i] = int(c * warehouse[i].singlePrice);
			}
			sum += pTicketTemp->credit[i];
		}

		// 支付
		do {
			bContinue = false;
			// 会员卡 or 现金
			do {
				ScanShort("请输入会员卡号（现金为-1）：", &pTicketTemp->vipCard, true);
				pUserTemp = GetCardById(pTicketTemp->vipCard);
			} while (pUserTemp == NULL && printf("没有找到该会员。\n"));

			// 收现金
			if (pTicketTemp->vipCard == -1)
			{
				do {
					ScanInt("请输入收入现金数量：", &pTicketTemp->given);
					bContinue = pTicketTemp->given < sum;
					if (bContinue) printf("不能倒贴钱。\n");
					if (bContinue && !ScanBoolean("是否更换收入现金数量？(y/n)：")) doClean(bContinue, false);
				} while (bContinue);
				if (pTicketTemp == NULL) break;
			}
			// 收会员卡
			else
			{
				while (pUserTemp->balance < sum)
				{
					if (ScanBoolean("余额不足，是否立刻充值？(y/n)：")) {
						ChargeInConsole(pTicketTemp->vipCard);
					} else {
						if (ScanBoolean("是否更换其他卡或使用现金购买？(y/n)：")) {
							bContinue = true;
						} else {
							printf("放弃充值，购买失败，退出中. . . \n");
							doClean(bContinue, false);
						} break;
					}
				}
				if (pTicketTemp == NULL) break;
				if (bContinue) continue;
				pTicketTemp->given = pUserTemp->balance;
			}

			pTicketTemp->left = pTicketTemp->given - sum;
		} while (bContinue);
		if (pTicketTemp == NULL) continue;
		
		ChargeToCard(pTicketTemp->vipCard, sum, false);
		pUserTemp = NULL;

		if (pTicketRear == NULL)
		{
			pTicketFront = pTicketTemp;
			pTicketRear = pTicketTemp;
		}
		else
		{
			pTicketRear->next = pTicketTemp;
			pTicketRear = pTicketTemp;
		}

		pTicketTemp = NULL;

	} while (ScanBoolean("是否继续新建订单？(y/n)："));
	
	return false;
}

#undef doClean

void OutputTicket(ticket* ticket, bool isFull) 
{
	if (isFull)
	{
		printf("==============================================\n");
		printf("|                水果超市票据                |\n");
	}
	printf("==============================================\n");
	printf("|                                            |\n");
	if (ticket->vipCard != -1)
	{
		pUserTemp = GetCardById(ticket->vipCard);
		printf("|  付款人：%-10s  付款卡：%04d          |\n", pUserTemp->name, pUserTemp->uid);
		pUserTemp = NULL;
	}
	printf("|  付款方式：%s    票据号：%04d          |\n", ticket->vipCard == -1 ? "现金  " : "会员卡", ticket->tid);
	int calc = int(ticket->time - pTime);
	printf("|  订购时间：%04d年%2d月%2d日 %2d:%02d            |\n", 
		pCurrentDate->tm_year + 1900, pCurrentDate->tm_mon + 1, pCurrentDate->tm_mday, calc / 3600, calc / 60 % 60);
	printf("|--------------------------------------------|\n");
	printf("|  # 商品名           单价    数量     金额  |\n");
	int sum = 0;
	double count = 0, credit = 0;
	for (int i = 0; i < 5; i++)
	{
		if (pTicketTemp->amount[i] == 0) continue;
		sum += pTicketTemp->credit[i];
		credit = dollar(pTicketTemp->credit[i]);
		count = dollar(pTicketTemp->amount[i] * (warehouse[i].isSingled ? 100 : 1));
		printf("|  %d %-12s  %7.2lf %7.2lf %8.2lf  |\n",
			i+1, warehouse[i].fruitName, credit / count, count, credit);
	}
	printf("|--------------------------------------------|\n");
	printf("|  合计                            %-8.2lf  |\n", dollar(sum));
	printf("|                                            |\n");
	if (ticket->vipCard == -1)
		printf("|  收款：%8.2lf      找零：%8.2lf        |\n", dollar(pTicketTemp->given), dollar(pTicketTemp->left));
	else
		printf("|  原有：%8.2lf      余额：%8.2lf        |\n", dollar(pTicketTemp->given), dollar(pTicketTemp->left));
	printf("|                                            |\n");
	if (isFull)
	{
		printf("|  1天退货保障  3天新鲜承诺   109店全院连锁  |\n");
		printf("|                                            |\n");
		printf("==============================================\n");
	}

}

void OutputAllTickets()
{
	printf("==============================================\n");
	printf("|                水果超市票据                |\n");
	pTicketTemp = pTicketFront->next;
	while (pTicketTemp != NULL)
	{
		OutputTicket(pTicketTemp, false);
		pTicketTemp = pTicketTemp->next;
	}
	printf("==============================================\n");
}

bool ModifyTicket(short tid)
{
	if (ScanBoolean("是否同时变动金额、库存等的改动？(y/n)："))
	{
		// TODO
	}
	else
	{
		// TODO
	}
	return false; 
}

bool DeleteTicket(short tid)
{
	if (ScanBoolean("是否撤销金额、库存等的改动？(y/n)："))
	{
		// TODO
	}
	else
	{
		// TODO
	}
	return false; 
}

void ExportTickets()
{
	// 生成文件名
	char pFileName[30];
	sprintf(pFileName, "%04d-%d-%d 导出消费记录.csv",
		pCurrentDate->tm_year + 1900, pCurrentDate->tm_mon + 1, pCurrentDate->tm_mday);

	// 打开文件
	FILE *pFile;
	do {
		pFile = fopen(pFileName, "w+");
		if (pFile == NULL)
		{
			printf("文件%s操作失败。\n", pFileName);
			if (ScanBoolean("是否重试？(y/n)：")) continue;
		}
		break;
	} while (true);
	if (pFile == NULL) return;

	fprintf(pFile, "票号,时间,付款人,卡号,合计,收款,找零");
	for (int i = 0; i < 5; i++)
	{
		fprintf(pFile, ",%s数量,%s金额", warehouse[i].fruitName, warehouse[i].fruitName);
	}
	fprintf(pFile, "\n");

	pTicketTemp = pTicketFront;
	while (pTicketTemp != NULL)
	{
		int calc = int(pTicketTemp->time - pTime);
		fprintf(pFile, "%04d,%d:%02d,", pTicketTemp->tid, calc / 3600, calc / 60 % 60);
		user *puser = GetCardById(pTicketTemp->vipCard);
		if (puser == NULL) fprintf(pFile, "已删除\t0000\t");
		else fprintf(pFile, "%s,%04d,", puser->name, puser->uid);
		fprintf(pFile, "%.2lf,%.2lf,%.2lf",
			dollar(pTicketTemp->given - pTicketTemp->left),
			dollar(pTicketTemp->given), dollar(pTicketTemp->left));
		for (int i = 0; i < 5; i++)
			if (warehouse[i].isSingled)
				fprintf(pFile, ",%d,%.2lf", pTicketTemp->amount[i], dollar(pTicketTemp->credit[i]));
			else
				fprintf(pFile, ",%.2lf,%.2lf", dollar(pTicketTemp->amount[i]), dollar(pTicketTemp->credit[i]));
		fprintf(pFile, "\n");
		pTicketTemp = pTicketTemp->next;
	}

	fclose(pFile);
	printf("记录已导出到“%s”。\n", pFileName);

}

void _ticket_test()
{
	clear();
	char op;
	while (true)
	{
		printf("==================\n");
		printf("|    小票系统\n");
		printf("==================\n");
		printf("|\n");
		printf("|   1.添加一张小票\n");
		printf("|   2.输出一张小票的信息\n");
		printf("|   3.按照号码查找小票\n");
		printf("|   4.修改小票内容\n");
		printf("|   5.删除小票\n");
		printf("|   6.查询大额购物信息\n");
		printf("|   7.查询某段时间内购物信息\n");
		printf("|   8.导出购物信息\n");
		printf("|   9.退出\n");
		printf("|\n");
		printf("==================\n");
		op = ScanOption("请选择进入：", '1', '9');
		printf("\n");
		switch (op)
		{
		case '1':
			AddTicket();
			sleep(500);
			break;
		case '2':
			short tid;
			ScanShort("请输入购物单号：", &tid, false);
			pTicketTemp = FindTicket(tid);
			if (pTicketTemp == NULL)
			{
				printf("没有找到该购物记录。\n");
			}
			else
			{
				OutputTicket(pTicketTemp, true);
				pTicketTemp = NULL;
			}
			pause();
			break;
		case '3':
			pause();
			break;
		case '4':
			sleep(500);
			break;
		case '5':
			sleep(500);
			break;
		case '6':
			sleep(500);
			break;
		case '7':
			sleep(500);
			break;
		case '8':
			ExportTickets();
			sleep(500);
			break;
		case '9':
			if (ScanBoolean("确定退出嘛(y/n)："))
				op = -52;
			break;
		default:
			break;
		}
		
		if (op == -52)
		{
			SaveTicketToFile();
			SaveUserToFile();
			SaveStockToFile();
			LoadStockFromFile();
			LoadUserFromFile();
			LoadTicketFromFile();
			sleep(500);
			clear();
			break;
		}
		clear();
	}
}