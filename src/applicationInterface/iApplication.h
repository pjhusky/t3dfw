#ifndef _iAPPLICATION_H_6635ff59_825e_41bd_9358_85cad169f5eb
#define _iAPPLICATION_H_6635ff59_825e_41bd_9358_85cad169f5eb

#include "../statusType.h"

struct ContextOpenGL;

struct iApplication {
    //Application( const ContextOpenGL& contextOpenGL );

    virtual Status_t run() = 0;
};

#endif // _iAPPLICATION_H_6635ff59_825e_41bd_9358_85cad169f5eb
