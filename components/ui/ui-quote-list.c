#include "ui-quote-list.h"

#define MAX_ROWS 6
#define COLW_TICKER 96
#define COLW_NUM 60
#define COLW_PRICE 108
#define ROW_H 48
#define COL_DIM 0x9AA0A6
#define HEADER_H 18

#define WHITE 0xFFFFFF
#define GREEN 0x2ECC71
#define RED 0xFF4D4F
#define ERROR 0x3A3F4B

typedef struct {
    lv_obj_t *row;
    lv_obj_t *sym, *name;
    lv_obj_t *prev, *open, *high, *low;
    lv_obj_t *price, *chg_box, *chg;
} quote_item_t;

static quote_item_t quote_list[MAX_ROWS];

// static lv_obj_t *make_label(lv_obj_t *parent, const lv_font_t *font)
// {
//     lv_obj_t *label = lv_label_create(parent);
//     lv_obj_set_style_text_font(label, font, 0);
//     lv_obj_set_style_text_color(label, lv_color_white(), 0);
//     return label;
// }

static lv_obj_t *make_container(lv_obj_t *row)
{
    lv_obj_t *o = lv_obj_create(row);
    lv_obj_set_size(o, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    return o;
}

static lv_obj_t *make_label(lv_obj_t *row, const lv_font_t *font,
                           uint32_t color, int width, lv_text_align_t align)
{
    lv_obj_t *label = lv_label_create(row);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_width(label, width);
    lv_obj_set_style_text_align(label, align, 0);
    return label;
}

static void set_num(lv_obj_t *label, float v)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f", v);
    lv_label_set_text(label, buf);
}

// SYMB / PREV / OPEN / HIGH / LOW / PRICE 
static void make_quote_header(lv_obj_t *list_screen)
{
    lv_obj_t *h = make_container(list_screen);
    lv_obj_set_size(h, lv_pct(100), HEADER_H);
    lv_obj_set_style_pad_hor(h, 8, 0);
    lv_obj_set_style_pad_column(h, 4, 0);
    lv_obj_set_flex_flow(h, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(h, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    const lv_font_t *f = &lv_font_montserrat_12;
    lv_label_set_text(make_label(h, f, COL_DIM, COLW_TICKER, LV_TEXT_ALIGN_LEFT),  "TICKER");
    lv_label_set_text(make_label(h, f, COL_DIM, COLW_NUM,    LV_TEXT_ALIGN_RIGHT), "PREV");
    lv_label_set_text(make_label(h, f, COL_DIM, COLW_NUM,    LV_TEXT_ALIGN_RIGHT), "OPEN");
    lv_label_set_text(make_label(h, f, COL_DIM, COLW_NUM,    LV_TEXT_ALIGN_RIGHT), "HIGH");
    lv_label_set_text(make_label(h, f, COL_DIM, COLW_NUM,    LV_TEXT_ALIGN_RIGHT), "LOW");
    lv_label_set_text(make_label(h, f, COL_DIM, COLW_PRICE,  LV_TEXT_ALIGN_RIGHT), "PRICE");
}

static void make_quote_list(lv_obj_t *list_screen, quote_item_t *quote_item)
{
    // the row itself (also the highlight)
    quote_item->row = lv_obj_create(list_screen);
    lv_obj_set_size(quote_item->row, lv_pct(100), ROW_H);
    lv_obj_set_style_bg_color(quote_item->row, lv_color_hex(0x2A2F3A), 0);   // highlight color
    lv_obj_set_style_bg_opa(quote_item->row, LV_OPA_TRANSP, 0);              // off by default
    lv_obj_set_style_radius(quote_item->row, 0, 0);
    lv_obj_set_style_border_side(quote_item->row, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(quote_item->row, 1, 0);
    lv_obj_set_style_border_color(quote_item->row, lv_color_hex(0x2A2F3A), 0);
    lv_obj_set_style_pad_hor(quote_item->row, 8, 0);
    lv_obj_set_style_pad_ver(quote_item->row, 0, 0);
    lv_obj_set_style_pad_column(quote_item->row, 4, 0);
    lv_obj_set_flex_flow(quote_item->row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(quote_item->row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // ticker cell: symbol on top, company name under it, centered in the row
    lv_obj_t *ticker = make_container(quote_item->row);
    lv_obj_set_size(ticker, COLW_TICKER, lv_pct(100));
    lv_obj_set_flex_flow(ticker, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ticker, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(ticker, 0, 0);

    quote_item->sym  = make_label(ticker, &lv_font_montserrat_20, WHITE,   COLW_TICKER, LV_TEXT_ALIGN_LEFT);
    quote_item->name = make_label(ticker, &lv_font_montserrat_12, COL_DIM, COLW_TICKER, LV_TEXT_ALIGN_LEFT);
    lv_obj_set_height(quote_item->name, 14);                      // one line only
    lv_label_set_long_mode(quote_item->name, LV_LABEL_LONG_DOT);  // long names end in "..."

    lv_obj_set_style_translate_y(quote_item->sym, 0, 0);          // symbol down 2 px
    lv_obj_set_style_translate_y(quote_item->name, -0, 0);        // name up 2 px

    // four number columns
    quote_item->prev = make_label(quote_item->row, &lv_font_montserrat_14, WHITE, COLW_NUM, LV_TEXT_ALIGN_RIGHT);
    quote_item->open = make_label(quote_item->row, &lv_font_montserrat_14, WHITE, COLW_NUM, LV_TEXT_ALIGN_RIGHT);
    quote_item->high = make_label(quote_item->row, &lv_font_montserrat_14, WHITE, COLW_NUM, LV_TEXT_ALIGN_RIGHT);
    quote_item->low  = make_label(quote_item->row, &lv_font_montserrat_14, WHITE, COLW_NUM, LV_TEXT_ALIGN_RIGHT);

    // price cell: price on top, colored change box under it
    lv_obj_t *price_cell = make_container(quote_item->row);
    lv_obj_set_width(price_cell, COLW_PRICE);
    lv_obj_set_flex_flow(price_cell, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(price_cell, 2, 0);

    quote_item->price = make_label(price_cell, &lv_font_montserrat_18, WHITE, COLW_PRICE, LV_TEXT_ALIGN_RIGHT);

    quote_item->chg_box = make_container(price_cell);
    lv_obj_set_width(quote_item->chg_box, COLW_PRICE);            // fixed width
    lv_obj_set_style_radius(quote_item->chg_box, 4, 0);
    lv_obj_set_style_bg_opa(quote_item->chg_box, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_ver(quote_item->chg_box, 2, 0);
    quote_item->chg = make_label(quote_item->chg_box, &lv_font_montserrat_12, WHITE, COLW_PRICE, LV_TEXT_ALIGN_CENTER);
}

static void update_quote_list(quote_item_t *quote_item, const char *symbol, const char *name,
                              const finnhub_quote_t *q, bool valid)
{
    lv_label_set_text(quote_item->sym, symbol);
    lv_label_set_text(quote_item->name, name ? name : "");

    if (!valid) {
        lv_label_set_text(quote_item->price, "-");
        lv_label_set_text(quote_item->prev, "-");
        lv_label_set_text(quote_item->open, "-");
        lv_label_set_text(quote_item->high, "-");
        lv_label_set_text(quote_item->low, "-");
        lv_label_set_text(quote_item->chg, "-");
        lv_obj_set_style_bg_color(quote_item->chg_box, lv_color_hex(ERROR), 0);
        return;
    }

    set_num(quote_item->price, q->current);
    set_num(quote_item->prev,  q->prev_close);
    set_num(quote_item->open,  q->open);
    set_num(quote_item->high,  q->high);
    set_num(quote_item->low,   q->low);

    char buf[28];
    snprintf(buf, sizeof(buf), "%+.2f (%+.2f%%)", q->change, q->percent_change);
    lv_label_set_text(quote_item->chg, buf);
    lv_obj_set_style_bg_color(quote_item->chg_box,
        lv_color_hex(q->change >= 0 ? GREEN : RED), 0);
}

void quote_list_create(lv_obj_t *list_screen)
{
    lv_obj_set_style_bg_color(list_screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_all(list_screen, 0, 0);
    lv_obj_set_style_pad_row(list_screen, 0, 0);
    lv_obj_set_flex_flow(list_screen, LV_FLEX_FLOW_COLUMN);

    make_quote_header(list_screen);
    for (int i = 0; i < MAX_ROWS; i++) {
        make_quote_list(list_screen, &quote_list[i]);
    }
}

void quote_list_update(const char *const *symbols, const char *const *names,
                       const finnhub_quote_t *q, const bool *valid,
                       size_t count, size_t selected)
{
    for (size_t i = 0; i < count && i < MAX_ROWS; i++) {
        update_quote_list(&quote_list[i], symbols[i], names[i], &q[i], valid[i]);
        lv_obj_set_style_bg_opa(quote_list[i].row,
            i == selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    }
}