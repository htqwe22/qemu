/**********************************************************************************
 * FILE : stack_frame.h
 * Description:
 * Author: Kevin He
 * Created On: 2025-01-06 , At 13:01:27
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef KV_STACK_FRAME_H
#define KV_STACK_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif



struct stack_frame
{
  struct stack_frame *fp; //x29 every point is 64bits 
  void *lr; 			// x30
};

const struct stack_frame *get_prev_frame(const struct stack_frame *fp);

void debug_callstack(void *fp);





#ifdef __cplusplus
}
#endif

#endif //stack_frame.h
