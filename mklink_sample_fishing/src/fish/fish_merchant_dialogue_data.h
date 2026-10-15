/* Generated from docs/merchant-dialogue.md. */
#ifndef FISH_MERCHANT_DIALOGUE_DATA_H
#define FISH_MERCHANT_DIALOGUE_DATA_H
typedef struct {unsigned char pose;const char *lines[2];} fish_merchant_dialogue_t;
static const fish_merchant_dialogue_t fish_merchant_dialogue[9]={
    {0,{"来啦，今天想钓点啥？","先看看，东西都在这儿。"}},
    {1,{"有收获啊，拿来瞧瞧。","这几条不错，正新鲜。"}},
    {3,{"嚯，这家伙可不轻！","船尾那条，够吃一阵了。"}},
    {1,{"好鱼！今晚有口福了。","钱收好，下回还来。"}},
    {2,{"趁手的家伙，拿好啦。","带上它，再去试试手。"}},
    {0,{"先钓些鱼，不着急。","还差一点，改天再来。"}},
    {0,{"桶里空啦，去湖上转转。","没鱼也行，坐会儿吧。"}},
    {3,{"这件你已经有了呀。","先把手里的用顺了。"}},
    {3,{"装不下啦，下回再买。","先用掉一些，别浪费。"}},
};
#endif
