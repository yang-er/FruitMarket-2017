#pragma once
#include "structs.h"

extern stock warehouse[5];

/// <summary>从文件中加载库存信息</summary>
bool LoadStockFromFile();

/// <summary>将水果库存信息保存到文件</summary>
bool SaveStockToFile();

/// <summary>输出所有水果的库存</summary>
void OutputStock();

/// <summary>进入仓库模式</summary>
void menu_stock();
