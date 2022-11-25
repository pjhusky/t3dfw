#ifndef _STATUS_TYPE_H_92b260f3_1af7_4f4f_86e7_3fbba5a67224
#define _STATUS_TYPE_H_92b260f3_1af7_4f4f_86e7_3fbba5a67224

#include <string>

struct Status_t {

    enum class eStatus {
        SUCCESS,
        FAILURE,
        NOT_SET,
    };

    static Status_t OK() { return Status_t{ eStatus::SUCCESS }; }
    static Status_t ERROR( const std::string& msg = "" ) { return Status_t{ eStatus::FAILURE, msg }; }

    Status_t() 
        : status( eStatus::NOT_SET ) {}

    Status_t( const eStatus status, const std::string msg = "" )
        : status( status )
        , msg( msg ) {}

    bool operator==( const Status_t& rhs ) const {
        return status == rhs.status;
    }
    
    bool operator!=( const Status_t& rhs ) const {
        return status != rhs.status;
    }

    /* const */ eStatus     status;
    /* const */ std::string msg;
};

#endif // _STATUS_TYPE_H_92b260f3_1af7_4f4f_86e7_3fbba5a67224
