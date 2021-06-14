#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "setting.h"
#include "utility.h"
#include "sorter.h"
#include "insertion-sorter.h"

enum {
    MAGIC_MAX = 64,
};
static char const MAGIC_TEXT[MAGIC_MAX] = "!!insertion sort state file!!";

typedef struct insertion_sorter_t {
    sorter_t base;

    int k;  
    int i;  
    int flag;
    int state;
    double doing_list[NUMS_SIZE_MAX];
} insertion_sorter_t;

static
void next_step(sorter_t *b) 
{
	if (!b || !(SORTER_SORTING == b->state || SORTER_PAUSED == b->state))
        return;

    b->frame_cnt = 0;

    insertion_sorter_t *me = container_of(b, insertion_sorter_t, base);
    
    int i = me->i;
    int k = me->k;
    double *nums = b->nums;
    double *doing_list = me->doing_list;
    
    if(i>=b->size){
    	b->state = SORTER_SORTED;
        return;
	}
    
    switch(me->state){
    	case 0:
    		doing_list[i] = nums[i];
    		k = i;
    		me->k = k;
    		me->i = i;
    		me->state = 1;
    		break;
    	case 1:
    		if(me->flag==1){
    			if(b->ascend){
    				if(doing_list[k]<nums[k-1] && k-1>=0){
    					swap(doing_list+k,doing_list+k-1);
    					swap(nums+k,nums+k-1);
    					k--;
					}
					else{
						me->state = 2;
					}	
				}
				else{
					if(doing_list[k]>nums[k-1] && k-1>=0){
    					swap(doing_list+k,doing_list+k-1);
    					swap(nums+k,nums+k-1);
    					k--;
					}
					else{
						me->state = 2;
					}
				}
    			
				me->k = k;
			}
			me->flag = -1*me->flag;
    		break;
    	case 2:
    		nums[k] = doing_list[k];
    		i++;
    		me->i = i;
    		me->state = 0;
    		break;
    	case 3:
    		doing_list[k] = nums[k];
    		me->state = 1;
    		break;
	}
}

static
void display(sorter_t *b, double x, double y, double w, double h)
{
    // 没有数据
    if (!b || b->state == SORTER_UNREADY) {
        sorter_display_unready(x, y, w, h);
        return;
    }

    insertion_sorter_t *me = container_of(b, insertion_sorter_t, base);

    // 刚准备好
    if (b->state == SORTER_READY) {
        me->i = me->k = 0;
        me->flag = 1;
    }

    // 可视化排序情况
    double margin = 0.2;
    x += margin;
    y += margin;
    w -= 2*margin;
    h -= 2*margin;
    double base_height = 0.5;   // 最小元素的高度
    double max_span = h - base_height; // 最大元素高度减去 base_height
    double delta = b->max - b->min;
    double unit_width = w / b->size;

    double mousex = 0, mousey = 1;
    int hover_idx = 0;
    double const *nums = b->nums;
    double *doing_list = me->doing_list;
    for (int i = 0; i < b->size; ++i) {
        double x1 = x + i*unit_width;
        double h1 = (base_height + (nums[i] - b->min)/delta * max_span)/2;
        char *color = "Dark Gray";
        if (me->state == 1 && i == me->k && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "white";
        else if (me->state == 0 && i == me->i && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "Red";
        else if (me->state == 1 && ((i == me->k-1 && me->flag==1) || (i == me->k+1 && i<=me->i && me->flag!=1)) && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "Green";
        else if (i<=me->i && me->i < b->size && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "Orange";
        else if (i >= me->i)
            color = "Light Gray";
        drawRectangleColor(x1+0.01, y, unit_width-0.01, h1, true, color);
        if (unit_width > 0.05)
        	if (me->state == 1 && i == me->k && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            continue;
            else
            drawRectangleColor(x1, y, unit_width, h1, false, "Black");
    }
    
    if(1 == me->state ){
    		double x2 = x + me->k*unit_width;
        	double h2 = (base_height + (doing_list[me->k] - b->min)/delta * max_span)/2;
        	double y2 = y + (base_height + max_span)/2 +0.05;
        	char *color = "Red";

        	drawRectangleColor(x2, y2, unit_width, h2, true, color);
        	if (unit_width > 0.05)
            	drawRectangleColor(x2, y2, unit_width, h2, false, "Black");
	}
}

static
void restart(sorter_t *b)
{
    if (!b || b->state == SORTER_UNREADY)
        return;
    insertion_sorter_t *me = container_of(b, insertion_sorter_t, base);
    sorter_restart(b);
    me->i = me->k = 0;
    me->state = 0;
    me->flag = 1;
}

static
void destroy(sorter_t *b)
{
    if (!b) return;
    sorter_deconstruct(b);
    insertion_sorter_t *me = container_of(b, insertion_sorter_t, base);
    free(me);
}

static
bool save_state(sorter_t *b, char const *ofile)
{
    if (!b || !ofile) return false;

    bool ret = true;
    insertion_sorter_t *me = container_of(b, insertion_sorter_t, base);
    FILE *ostream = fopen(ofile, "w");
    if (!ostream) return false;

    if (fprintf(ostream, "%s\n", MAGIC_TEXT) < 0) {
        ret = false;
        goto closestream;
    }

    if (!sorter_save_state_at(b, ostream)) {
        ret = false;
        goto closestream;
    }

    fprintf(ostream, "%d %d\n", me->i, me->k);
    for (int i = 0; i <= me->k; i++) {
        if (0 != i) fprintf(ostream, " ");
        fprintf(ostream, "%d", me->doing_list[i]);
    }
    fprintf(ostream, "\n");
    if(me->state == 1){
    	fprintf(ostream, "%d\n", me->state+2);
	}
	else{
		fprintf(ostream, "%d\n", me->state);
	}
    fprintf(ostream, "%d\n", me->flag);

closestream:
    fclose(ostream);
    return ret;
}

sorter_t *insertion_sorter_create(void)
{
    insertion_sorter_t *me = malloc(sizeof(*me));
    if (!me) return NULL;

    sorter_construct(&me->base);
    me->base.next_step = next_step;
    me->base.display = display;
    me->base.restart = restart;
    me->base.destroy = destroy;
    me->base.save_state = save_state;

    me->i = me->k = 0;
    me->state = 0;
    me->flag = 1;

    return &me->base;
}

sorter_t *insertion_sorter_load(char const *state_file)
{
    if (!state_file) return NULL;
    insertion_sorter_t *me = malloc(sizeof(*me));
    if (!me) return NULL;

    FILE *istream = fopen(state_file, "r");
    if (!istream)
        goto freeme;

    char magic_buff[MAGIC_MAX] = "";
    if (!fgets(magic_buff, MAGIC_MAX-1, istream))
        goto closestream;
    /* 减一是为了移除\n */
    if (0 != strncmp(MAGIC_TEXT, magic_buff, strlen(magic_buff)-1))
        goto closestream;
    if (!sorter_load_from(&me->base, istream))
        goto closestream;
    if (2 != fscanf(istream, "%d %d", &me->i, &me->k))
        goto closestream;
    for (int i = 0; i <= me->k; i++) {
        if (1 != fscanf(istream, "%d", &me->doing_list[i]) 
                || me->doing_list[i] < 0 ) {
            goto closestream;
        }
    }
    if (1 != fscanf(istream, "%d", &me->state))
        goto closestream;
    if (1 != fscanf(istream, "%d", &me->flag))
        goto closestream;

    me->base.next_step = next_step;
    me->base.display = display;
    me->base.restart = restart;
    me->base.destroy = destroy;
    me->base.save_state = save_state;

    return &me->base;

closestream:
    fclose(istream);
freeme:
	
    free(me);
    return NULL;
}
