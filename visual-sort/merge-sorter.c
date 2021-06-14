#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "imgui.h"
#include "graphics.h"
#include "extgraph.h"

#include "setting.h"
#include "utility.h"
#include "sorter.h"
#include "merge-sorter.h"

enum {
    MAGIC_MAX = 64,
};
static char const MAGIC_TEXT[MAGIC_MAX] = "!!merge sort state file!!";

enum {
    MERGE_STEP1 = 0,
    MERGE_STEP2,
    MERGE_STEP3,
};

typedef struct merge_sorter_t {
	sorter_t base;
	
	int doing_size;
	double doing_list[NUMS_SIZE_MAX];
	int start, end, mid;
	int state;
	int i, j, k;
} merge_sorter_t;

static
int Check(double Arr[],int StartIndex, int EndIndex, bool flag);

static
void next_step(sorter_t *b) 
{
	if (!b || !(SORTER_SORTING == b->state || SORTER_PAUSED == b->state))
        return;

    b->frame_cnt = 0;

    merge_sorter_t *me = container_of(b, merge_sorter_t, base);
    
    bool flag = b->ascend;
    int size = b->size;
    int start = me->start;
    int end = me->end;
    int mid = me->mid;
    double *doing_list = me->doing_list;
    int i = me->i;
    int j = me->j;
    int k = me->k;
    double *nums = b->nums;
    
    switch(me->state){
    	case MERGE_STEP1:
    		start=0, end=size-1, mid=(end-start)/2+start;
	
			if(Check(nums, start, end, flag)){
				b->state = SORTER_SORTED;
				break;
			}
			while((Check(nums, start, mid, flag)==0 
			      || Check(nums, mid+1, end, flag)==0)&&(end-start)>1){
				if(Check(nums, start, mid, flag)==1){
					start=mid+1;
					mid=(end-start)/2+start;
				}else{
					end = mid;
					mid=(end-start)/2+start;
				}
			}
			me->i = me->k = me->start = start;
			me->end = end;
			me->mid = mid;
			me->j = mid +1;
			me->doing_size = start - end + 1; 
			me->state = MERGE_STEP2;
			break;
		case MERGE_STEP2:
		    i = me->i, j = me->j, k = me->k;
	
			if(i<=mid && j<=end){
				if(flag){
					if(nums[i]<nums[j]){
						doing_list[k++] = nums[i++];
					}
					else{
						doing_list[k++] = nums[j++];
					}
				}else{
					if(nums[i]>nums[j]){
						doing_list[k++] = nums[i++];
					}		
					else{
						doing_list[k++] = nums[j++];
					}
				}
			}
			else if(i<=mid){
				doing_list[k++] = nums[i++];
			}
			else if(j<=end){
				doing_list[k++] = nums[j++];
			}
			if(i>mid && j>end){
				me->state = MERGE_STEP3;
			}
			me->i = i;
			me->j = j;
			me->k = k;
			break;
		case MERGE_STEP3:
			for(i=start;i<=end;i++){
				nums[i] = doing_list[i];
			}
			me->state = MERGE_STEP1;
			if(Check(nums, 0, b->size-1, flag)){
				me->start = b->size;
			}
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

    merge_sorter_t *me = container_of(b, merge_sorter_t, base);

    // 刚准备好
    if (b->state == SORTER_READY) {
        me->i = me->j = me->k = 0;
        me->start = me->end = me->mid = 0;
    }

    // 可视化排序情况
    double margin = 0.2;
    x += margin;
    y += margin-0.05;
    w -= 2*margin;
    h -= 2*margin;
    double base_height = 0.5;   // 最小元素的高度
    double max_span = h - base_height; // 最大元素高度减去 base_height
    double delta = b->max - b->min;
    double unit_width = w / b->size;

    double mousex = 0, mousey = 1;
    int hover_idx = 0;
    double const *nums = b->nums;
    double const *doing_list = me->doing_list;
    
    for (int i = 0; i < b->size; i++) {
        double x1 = x + i*unit_width;
        double h1 = (base_height + (nums[i] - b->min)/delta * max_span)/2;
        char *color = "dark Gray";
        if (MERGE_STEP2 == me->state && (i == me->i && i<=me->mid)
		         && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "Red";
        else if ((MERGE_STEP2 == me->state||MERGE_STEP3 == me->state ) && ((i == me->i-1 &&i == me->mid)||(i < me->i && i<=me->mid && i>=me->start))
		         && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "white";
        else if (MERGE_STEP2 == me->state && (i == me->j && i<=me->end)
		         && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "Green";
        else if ((MERGE_STEP2 == me->state||MERGE_STEP3 == me->state ) && ((i == me->j-1 &&i == me->end)||(i < me->j && i>=me->mid+1 && i<=me->end))
		         && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "white";
        else if ((i > me->end && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED)) 
				 || b->state == SORTER_READY)
            color = "light Gray";
        else if (i >= me->start && i <= me->end && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            color = "Orange";
        drawRectangleColor(x1+0.01, y, unit_width-0.01, h1, true, color);
        
        if (unit_width > 0.05)
            if ((MERGE_STEP2 == me->state||MERGE_STEP3 == me->state ) && (((i == me->i-1 &&i == me->mid)||(i < me->i && i<=me->mid && i>=me->start))
                || ((i == me->j-1 &&i == me->end)||(i < me->j && i>=me->mid+1 && i<=me->end)))
		        && (b->state == SORTER_SORTING || b->state == SORTER_PAUSED))
            continue;
            else
			drawRectangleColor(x1, y, unit_width, h1, false, "Black");
    }
    
    if(MERGE_STEP2 == me->state||MERGE_STEP3 == me->state ){
    	for(int i = me->start;i < me->k;i++){
    		double x2 = x + i*unit_width;
        	double h2 = (base_height + (doing_list[i] - b->min)/delta * max_span)/2;
        	double y2 = y + (base_height + max_span)/2 +0.05;
        	char *color = "Orange";

        	drawRectangleColor(x2, y2, unit_width, h2, true, color);
        	if (unit_width > 0.05)
            	drawRectangleColor(x2, y2, unit_width, h2, false, "Black");
		}
	}
}

static
void restart(sorter_t *b)
{
    if (!b || b->state == SORTER_UNREADY)
        return;
    merge_sorter_t *me = container_of(b, merge_sorter_t, base);
    sorter_restart(b);
    me->i = me->k = me->k = 0;
    me->state = 0;
}

static
void destroy(sorter_t *b)
{
    if (!b) return;
    sorter_deconstruct(b);
    merge_sorter_t *me = container_of(b, merge_sorter_t, base);
    free(me);
}

static
int Check(double Arr[],int StartIndex, int EndIndex, bool flag)
{
	int i,j; 
	
	if(flag){
		for(i=StartIndex;i<EndIndex;i++){
			if(Arr[i]>Arr[i+1]){
				return 0;
			}
		}
		return 1;
	}else{
		for(i=StartIndex;i<EndIndex;i++){
			if(Arr[i]<Arr[i+1]){
				return 0;
			}
		}
		return 1;
	}
}

static
bool save_state(sorter_t *b, char const *ofile)
{
	if (!b || !ofile) return false;

    bool ret = true;
    merge_sorter_t *me = container_of(b, merge_sorter_t, base);
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
    if(me->state == MERGE_STEP2){
    	fprintf(ostream, "%d\n", me->state-1);
	}
	else{
		fprintf(ostream, "%d\n", me->state);
	}
    fprintf(ostream, "%d %d %d\n", me->start,me->end,me->mid);
    for (int i = 0; i <= me->end; i++) {
        if (0 != i) fprintf(ostream, " ");
        fprintf(ostream, "%d", me->doing_list[i]);
    }
    if (me->end > me->start) fprintf(ostream, "\n");
    
    fprintf(ostream, "%d %d %d\n", me->i,me->j,me->k);
    
closestream:
    fclose(ostream);
    return ret;
}

sorter_t *merge_sorter_create(void)
{
    merge_sorter_t *me = malloc(sizeof(*me));
    if (!me) return NULL;

    sorter_construct(&me->base);
    me->base.next_step = next_step;
    me->base.display = display;
    me->base.restart = restart;
    me->base.destroy = destroy;
    me->base.save_state = save_state;

    me->i = me->k = me->k = 0;
    me->state = 0;

    return &me->base;
}

sorter_t *merge_sorter_load(char const *state_file)
{
	if (!state_file) return NULL;
    merge_sorter_t *me = malloc(sizeof(*me));
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
        
    if (1 != fscanf(istream, "%d", &me->state))
        goto closestream;
    if (3 != fscanf(istream, "%d %d %d", &me->start, &me->end, &me->mid))
        goto closestream;
    for (int i = 0; i <= me->end; i++) {
        if (1 != fscanf(istream, "%d", &me->doing_list[i]) 
                || me->doing_list[i] <= 0 ) {
            goto closestream;
        }
    }
    if (3 != fscanf(istream, "%d %d %d", &me->i, &me->j, &me->k))
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
