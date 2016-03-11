struct NEWORDER 
{
	/* Input data. */
	int w_id;
	int d_id;
	int c_id;
	int ol_cnt;
	int all_local;
	int itemid[15];
	int supware[15];
	int qty[15];
};

union TXN_DATA {
	struct NEWORDER neworder;
};

struct TXN {
	int txn_type;
	union TXN_DATA txn_data;
};
