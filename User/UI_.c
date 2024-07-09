/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-06 18:52:10
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-07-07 13:02:59
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\User\UI_.c
 * @Description:
 *
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved.
 */

#include "UI_.h"
#include "OLED.h"
#include "oled_buffer.h"
#include "string.h"
#include "oled_draw.h"

struct Page_UI_t
{
	char *name;
	struct Page_UI_t *last_page;
	struct Item_UI_t *item_list;
	int item_num;
} *current_page;

int current_item_index = 0;

struct Item_UI_t
{
	char *name;
	void (*func)(void);
	struct Page_UI_t *next_page;
	struct Item_UI_t *next;
	struct Item_UI_t *prev;
} *current_item;

void UI_back(void)
{
	if (current_page->last_page != NULL)
	{
		current_page = current_page->last_page;
		current_item = current_page->item_list;
		current_item_index = 0;
		UI_updata();
	}
}

void UI_next(void)
{
	// switch the cursor
	if (current_item->next != NULL)
	{
		DrawChar(current_item_index, 0, ' ');
		current_item = current_item->next;
		current_item_index++;
		DrawChar(current_item_index, 0, '>');
	}
}

void UI_prev(void)
{
	// switch the cursor
	if (current_item->prev != NULL)
	{
		DrawChar(current_item_index, 0, ' ');
		current_item = current_item->prev;
		current_item_index--;
		DrawChar(current_item_index, 0, '>');
	}
}

void UI_enter(void)
{
	if (current_item->func != NULL)
	{
		current_item->func();
	}
	if (current_item->next_page != NULL)
	{
		current_page = current_item->next_page;
		current_item = current_page->item_list;
		current_item_index = 0;
		UI_updata();
	}
}

void UI_updata(void)
{
	ClearScreenBuffer(0x00);
	// first column is the item that is selected
	DrawChar(current_item_index, 0, '>');
	DrawString(current_item_index, 2, current_page->item_list[current_item_index].name);
	// third column is the item that is not selected
	for (int i = 0; i < current_page->item_num; i++)
	{
		if (i != current_item_index)
		{
			DrawString(i, 2, current_page->item_list[i].name);
		}
	}
}

/// @brief test function
struct Page_UI_t page1;
struct Page_UI_t page2;
struct Page_UI_t page3;

void get_start(void)
{
	// turn on led
	taskENTER_CRITICAL();
	main_task_init();
	taskEXIT_CRITICAL();
}

void get_stop(void)
{
	// turn off led
	taskENTER_CRITICAL();
	main_task_deinit();
	taskEXIT_CRITICAL();
}

void turn_on_led1(void)
{
	// turn on led1
	GPIO_SetBits(GPIOE, GPIO_Pin_1);
}

void turn_off_led1(void)
{
	// turn off led1
	GPIO_ResetBits(GPIOE, GPIO_Pin_1);
}

void test1(void)
{
	// 3 pages
	// first page
	page1.name = "page1";
	page1.last_page = NULL;
	page1.item_num = 2;
	page1.item_list = (struct Item_UI_t *)malloc(sizeof(struct Item_UI_t) * 2);

	page1.item_list[0].name = "item1";
	page1.item_list[0].func = NULL;
	page1.item_list[0].next_page = &page2;
	page1.item_list[0].next = &page1.item_list[1];
	page1.item_list[0].prev = NULL;

	page1.item_list[1].name = "item2";
	page1.item_list[1].func = NULL;
	page1.item_list[1].next_page = &page3;
	page1.item_list[1].next = NULL;
	page1.item_list[1].prev = &page1.item_list[0];

	// second page
	page2.name = "page2";
	page2.last_page = &page1;
	page2.item_num = 2;
	page2.item_list = (struct Item_UI_t *)malloc(sizeof(struct Item_UI_t) * 2);

	page2.item_list[0].name = "start";
	page2.item_list[0].func = get_start;
	page2.item_list[0].next_page = NULL;
	page2.item_list[0].next = &page2.item_list[1];
	page2.item_list[0].prev = NULL;

	page2.item_list[1].name = "stop";
	page2.item_list[1].func = get_stop;
	page2.item_list[1].next_page = NULL;
	page2.item_list[1].next = NULL;
	page2.item_list[1].prev = &page2.item_list[0];

	// third page
	page3.name = "page3";
	page3.last_page = &page1;
	page3.item_num = 2;
	page3.item_list = (struct Item_UI_t *)malloc(sizeof(struct Item_UI_t) * 2);

	page3.item_list[0].name = "turn on led1";
	page3.item_list[0].func = turn_on_led1;
	page3.item_list[0].next_page = NULL;
	page3.item_list[0].next = &page3.item_list[1];
	page3.item_list[0].prev = NULL;

	page3.item_list[1].name = "turn off led1";
	page3.item_list[1].func = turn_off_led1;
	page3.item_list[1].next_page = NULL;
	page3.item_list[1].next = NULL;
	page3.item_list[1].prev = &page3.item_list[0];
	
	
	current_page=&page1;
	current_item=&page1.item_list[0];
}
