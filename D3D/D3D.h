#ifndef __D3D__
#define __D3D__

#pragma pack(push, 1)

#define D3__DLL_ORG_ADDRESS 0x00800000 //binary too large, should not rebase
#define D3__DLL_CUR_ADDRESS 0x00800000
#define D3__FUNC(type, style, offset, args) ((type (style *)args)(D3__DLL_CUR_ADDRESS + (offset - D3__DLL_ORG_ADDRESS)))
#define D3__VAR(type, offset) ((type)(D3__DLL_CUR_ADDRESS + (offset - D3__DLL_ORG_ADDRESS)))
#define D3__VAR_DEREF(type, offset) ((type)(*(unsigned long*)(D3__DLL_CUR_ADDRESS + (offset - D3__DLL_ORG_ADDRESS))))

#define D3_GetGlobal() D3__VAR_DEREF(D3_GLOBAL_PTR, 0x18939C4)
#define D3_GetTLS() (&D3_GetGlobal()->tls)
#define D3_GetAttr_AsInt D3__FUNC(int, __thiscall, 0xEB48F0, (D3_ACD_PTR acd, unsigned long attr_id)) //strength
#define D3_GetAttr_AsFloat D3__FUNC(float, __thiscall, 0xEB4830, (D3_ACD_PTR acd, unsigned long attr_id))
#define D3_GetGold D3__FUNC(unsigned long long, __thiscall, 0xEABFB0, (D3_ACD_PTR acd)) //acdinventorygetgold
#define D3_GetSquare D3__FUNC(unsigned long*, __cdecl, 0xEE51B0, (D3_ACD_PTR acd, unsigned long slot_id, unsigned long *pos))
#define D3_GetSlotSize D3__FUNC(void, __cdecl, 0xEE1010, (unsigned long slot_id, unsigned long *pos))
#define D3_GetCurACDHid D3__FUNC(unsigned long, __cdecl, 0xA40B50, ())
#define D3_GetACD D3__FUNC(D3_ACD_PTR, __cdecl, 0x94A290, (unsigned long acd_hid))
#define D3_GetActor D3__FUNC(D3_ACTOR_PTR, __cdecl, 0x926250, (unsigned long actor_hid))
#define D3_HasUI D3__FUNC(D3_UI_PTR, __cdecl, 0x8F18A0, (unsigned long long *ui_hid))
#define D3_GetUI D3__FUNC(D3_UI_PTR, __cdecl, 0x8F1910, (unsigned long long *ui_hid)) //UI_GetControlFromID
#define D3_GetPlayer D3__FUNC(void*, __cdecl, 0x00EB63E0, (unsigned long acd_hid))

#define D3_CreateACD D3__FUNC(unsigned long, __cdecl, 0xED43E0, (unsigned long bnet_hid, void *generator)) //Tried to create an item (%s) with an invalid actor!
#define D3_DeleteACD D3__FUNC(void, __cdecl, 0xEB2D80, (unsigned long acd_hid)) //Tried to create an item (%s) with an invalid actor!

#define D3_IsLargeItem D3__FUNC(int, __cdecl, 0xED26D0, (unsigned long gb_hid)) //Failed to get item %s.  A treasure clas

//uiclick
#define D3_GetSkillErrorCode D3__FUNC(int, __cdecl, 0xEC9840, (void *acd, unsigned long skill_id, void *evt))
typedef struct
{
	void *_00[200];
	unsigned int x;
	unsigned int y;
} D3_GAME_DATA, *D3_GAME_DATA_PTR;
#define D3_GameData() D3__VAR(D3_GAME_DATA_PTR, 0x1800F44)
typedef struct
{
	unsigned int actor_hid;
	unsigned int skill_id_0;
	unsigned int skill_id_1;
	unsigned int flag;
	unsigned int other_hid;
} D3_USESKILL_DATA, *D3_USESKILL_DATA_PTR;
#define D3_UseSkill D3__FUNC(int, __cdecl, 0x94D740, (void *actor, void *useskill_data, int _08_1, int _0C_0, int _10_0))

//Missing item attribute label
typedef struct
{
	struct {
		short ref_count;
		short alloc_type;
		long size;
		long size_limit;
		void *obj;
		unsigned long sig;
	} *hdr;
	char *s;
	unsigned int flag;
} D3_STRING, *D3_STRING_PTR;
#define D3_StringInit D3__FUNC(void, __thiscall, 0x1019520, (D3_STRING_PTR ecx))
#define D3_StringDel D3__FUNC(void, __thiscall, 0x1019D30, (D3_STRING_PTR ecx))
#define D3_GetLang D3__FUNC(int, __cdecl, 0xE7E260, (int tid, int cid, D3_STRING_PTR s))
#define D3_GetItemName D3__FUNC(D3_STRING_PTR, __cdecl, 0xBE3FA0, (D3_STRING_PTR s, D3_ACD_PTR acd, int flag, int ident_flag))

//Hook MainLoop
#define D3__PEEKMESSAGE D3__VAR(unsigned long*, 0x01437A78)
#define D3__PEEKMESSAGE_CALL_RET D3__VAR(unsigned long, 0x00E5B94B)
void D3_Hook_Mainloop_Setup();
void D3_Mainloop();
//--
//Hook Server Packet
#define D3__SERVER_PACKET_HANDLER() D3__VAR(unsigned long*, 0x188E270) //Error Parsing ProtocolBufferMessage
void D3_Hook_OnServerPacket_Setup();
int D3_OnServerPacket(void *packet, int type, void *global_38);
//--

typedef struct
{
	unsigned long attr_id;
	unsigned long default_val;
	unsigned long mask;
	void *_0C;
	int is_int;
	void *_14[2];
	const char *attr_name;
	void *obj;
	unsigned char flag;
} D3_ATTR_DESC, *D3_ATTR_DESC_PTR;
#define D3_GetAttrDescTable() D3__VAR(D3_ATTR_DESC_PTR, 0x016AD820)
#define D3_GetAttrDescTableSIZE() (D3__VAR(D3_ATTR_DESC_PTR, 0x016B6218) - D3_GetAttrDescTable())

//Missing item attribute label
typedef struct
{
	unsigned long hid;
	unsigned long attr_id_0;
	unsigned long attr_id_1;
	unsigned long attr_id_2;
	void *_10;
	void *_14;
	void *call_has_affix; //(desc, acd_hid)
	void *call_format;
	void *_20;
} D3_ITEM_AFFIX_DESC, *D3_ITEM_AFFIX_DESC_PTR;
#define D3_GetItemAffixDescTable() D3__VAR(D3_ITEM_AFFIX_DESC_PTR, 0x1858AE8)
#define D3_GetItemAffixDescTableSize() (D3__VAR(D3_ITEM_AFFIX_DESC_PTR, 0x185ACCC) - D3_GetItemAffixDescTable())
#define D3_GetItemAffixDescExTable() D3__VAR(D3_ITEM_AFFIX_DESC_PTR, 0x185ACCC)
#define D3_GetItemAffixDescExTableSize() (D3__VAR(D3_ITEM_AFFIX_DESC_PTR, 0x185ADA4) - D3_GetItemAffixDescExTable())


typedef struct
{
	char *name;
	unsigned int hid;
	void *handler;
} D3_UI_HANDLER_DESC, *D3_UI_HANDLER_DESC_PTR;
#define D3_GetUIHandlerDescTable() D3__VAR(D3_UI_HANDLER_DESC_PTR, 0x016A3E78)
#define D3_GetUIHandlerDescTableSize() (D3__VAR(D3_UI_HANDLER_DESC_PTR, 0x016A6890) - D3_GetUIHandlerDescTable())

typedef struct
{
	union {
		struct {
			float i;
			float j;
			float k;
			float z;
		};
		struct {
			float x;
			float y;
			float w;
			float h;
		};
		struct {
			float p0_x;
			float p0_y;
			float p1_x;
			float p1_y;
		};
	};
} D3_FLOAT_POINT_4, *D3_FLOAT_POINT_4_PTR;
typedef struct
{
	float x;
	float y;
} D3_FLOAT_POINT_2, *D3_FLOAT_POINT_2_PTR;
typedef struct
{
	float x;
	float y;
	float z;
} D3_FLOAT_POINT_3, *D3_FLOAT_POINT_3_PTR;

typedef struct
{
	union {
		void **ptr;
		void *ptr_ip;
	};
	void *_04;
	int sz;
	void *_0C;
	struct {
		void **ptr;
		void *_04;
		int sz; //0x08
		void *_0C[3];
		unsigned int signature; //0x18 - 600DF00D
	} _alloc; //0x10;
} D3_SIMPLE_LIST, *D3_SIMPLE_LIST_PTR;


typedef struct
{
	unsigned long acd_hid; //0x00
	char name[0x80]; //0x04
	void *_84[1];
	unsigned int bnet_hid; //0x88
	unsigned int actor_hid; //0x8C
	void *_90[8];
	unsigned int type; //0xB0
	unsigned int gb_hid; //0xB4
	void *_B8;
	unsigned int is_owned; //0xBC
	D3_FLOAT_POINT_4 face_direction; //0xC0, <i, j, sin, cos>
	D3_FLOAT_POINT_4 pt0;//0xD0
	D3_FLOAT_POINT_4 pt1;//0xE0
	D3_FLOAT_POINT_4 pt2;//0xF0
	void *_100[2];
	unsigned int world_hid;
	unsigned int scene_hid;
	unsigned long acd_hid_owner;//0x110
	unsigned long slot_id;
	unsigned long slot_x;
	unsigned long slot_y;
	unsigned long attr_hid; //0x120
	void *_124[2];
	void *inventory; //0x12C
	void *_130[13];
	unsigned long is_unident; //x164
	void *_168[90];

	//0x1C4 list?

} D3_ACD, *D3_ACD_PTR;

typedef struct
{
	char name[0x100];
	unsigned long limit; //0x100
	unsigned long _104;
	unsigned long largest_idx; //0x108
	unsigned long count;
	unsigned long seq; //0x110
	void *_114[13]; //(0x11C,free objs count), (0x114 head, 0x118 tail,free list ptr)
	D3_SIMPLE_LIST slot_list; //0x148
	void *_174[4];
	unsigned long num_elem_per_slot; //0x184
	void *_188;
	unsigned char mask; //0x18C
} D3_HYBRID_LIST, *D3_HYBRID_LIST_PTR;
typedef D3_HYBRID_LIST D3_OBJECT_MANAGER, *D3_OBJECT_MANAGER_PTR;

typedef struct _D3_LIST
{
	_D3_LIST* next;
	_D3_LIST* prev;
	char data[1];
} D3_LIST, *D3_LIST_PTR;

typedef struct
{
	void *_00[2];
	D3_LIST_PTR *list_array;
	void *_0C;
	int list_array_size;
	void *_14[11];
	unsigned int mask; //0x40

} D3_LIST_MANAGER, *D3_LIST_MANAGER_PTR;

typedef struct
{

} D3_MOVEMENT, *D3_MOVEMENT_PTR;

typedef struct
{
	unsigned int hid;
	unsigned int world_hid;
	void *_08;
	unsigned int tile_x;
	unsigned int tile_y;
	void *_14[90];

	struct {
		unsigned int world_hid;
		unsigned int hid;
		unsigned int sno_id;
		D3_FLOAT_POINT_2 _pt;
		void *_14;
		D3_FLOAT_POINT_2 spt;
		D3_FLOAT_POINT_2 ept;

	} *_17C;

} D3_SCENE, *D3_SCENE_PTR;
typedef struct
{
	unsigned int tile_sz_x;
	unsigned int tile_sz_y;
	unsigned int total_tiles_x;
	unsigned int total_tiles_y;
	unsigned int offset_x; // (pos_x * 0.4 / tile_sz_x) - offset_x
	unsigned int offset_y; // (pos_y * 0.4 / tile_sz_y) - offset_y
	unsigned int *scene_hids;
} D3_NAV_MESH, *D3_NAV_MESH_PTR;
typedef struct
{
	void *_00;
	unsigned int hid;
	D3_NAV_MESH_PTR nav_mesh;

} D3_WORLD, *D3_WORLD_PTR;
#define D3_GetWorld D3__FUNC(D3_WORLD_PTR, __cdecl, 0xF12880, (unsigned int world_hid))
#define D3_GetScene D3__FUNC(D3_SCENE_PTR, __cdecl, 0xF13650, (unsigned int scene_hid))

typedef struct
{
	void *_00[2];
	struct NAV {
		D3_FLOAT_POINT_3 p0;
		D3_FLOAT_POINT_3 p1;
		unsigned short flag;
		unsigned short len;
		unsigned int near_idx;
	} *nav_lst; //0x08
	void *_0C[5];
	struct NEAR_NAVID {
		unsigned short flag;
		unsigned short nav_id;
	} *near_nav_id_lst; //0x20
	void *_24[3];
	float scale_div; //0x30 - 10
	float scale_mul; //0x34 - 0.1
	void *_38;
	unsigned int max_x; //0x3C
	unsigned int max_y; //0x40
	void *_44;
	struct IDX {
		unsigned short flag;
		unsigned short len;
		unsigned short idx;
	} *IDX_lst; //0x48
	void *_4C[1 + 4];
	struct NAVID {
		unsigned short flag;
		unsigned short nav_id;
	} *nav_id_lst; //0x60

} D3_NAVCELL_MGR, *D3_NAVCELL_MGR_PTR;
typedef struct
{
	unsigned int sno_id;
	void *_04[3];
	D3_FLOAT_POINT_3 pt[4];
	void *_40[10];
	char full_name[256]; //0x68
	void *_168[70];
	D3_NAVCELL_MGR mgr; //0x280
} D3_NAVCELL_SNO_MGR, *D3_NAVCELL_SNO_MGR_PTR;
#define D3_GetNavCellIdx2 D3__FUNC(void, __cdecl, 0x00FB6240, (void *scene, void *pos, void *idx))
#define D3_GetNavCellIdx D3__FUNC(int, __cdecl, 0x00FB6500, (void *pos, void *idx))


//Position: screen to Game 0xBE0050
#define D3_Pos_WorldToScreen D3__FUNC(int, __cdecl, 0x009E7250, (D3_FLOAT_POINT_4_PTR game_pos, D3_FLOAT_POINT_4_PTR screen_pos))
#define D3_Pos_ScreenToUI D3__FUNC(void, __cdecl, 0x0097D550, (D3_FLOAT_POINT_4_PTR screen_pos, D3_FLOAT_POINT_4_PTR ui_pos))

typedef struct
{
	unsigned long actor_hid;
	unsigned long acd_hid;
	char name[0x80];
	void *_88[2];
	D3_FLOAT_POINT_4 face_direction; //0x90, <i, j, sin, cos>
	D3_FLOAT_POINT_4 pt0;//0xA0
	D3_FLOAT_POINT_4 pt1;//0xB0
	D3_FLOAT_POINT_4 pt2;//0xC0 //0xCC tall?

	//D3_MOVEMENT_PTR movement; //0x384

} D3_ACTOR, *D3_ACTOR_PTR;

typedef struct
{
	unsigned long long ui_hid;
	char name[0x200];
} D3_UI_INFO, *D3_UI_INFO_PTR;

//AuctionHouseList.ItemListContainer.ItemList.ShowMoreItem
typedef struct
{
	void *_04[2];
	D3_UI_INFO info;
	void *_210[723];
	void *prev; //0xD5C
	union INNER {
		struct SEARCH_PAGE_DATA {
			void *_00[3];
			int num_per_page;
			void *_10[6];
			D3_SIMPLE_LIST item_list;
			void *_54[3];
			int page_idx;
			int total_items;
		} search;
		struct OTHER_DATA {
			void *_00[6];
			D3_SIMPLE_LIST item_list;
		} other;
	} *data; //0xD60
} D3_UI_ITEM_LIST_CONTAINER, *D3_UI_ITEM_LIST_CONTAINER_PTR;

#define D3_UI_EVENT_MOUSE_UP 7
#define D3_UI_EVENT_MOUSE_DOWN 6
//sz-0x48
typedef struct
{
	unsigned long evt_id;
	void *_04[0x11];
} D3_UI_EVENT, *D3_UI_EVENT_PTR;
#define D3_UI_InitEvent D3__FUNC(void, __thiscall, 0x9E3BA0, (D3_UI_EVENT_PTR evt, void *ptr))
typedef struct
{
	struct {
		void *_00;
		int (__thiscall *unk)(void *ecx); //0x04
		void *_08[3];
		void (__thiscall *SetPosRelative)(void *ecx, float*); //0x14
		void *_18[2];
		void (__thiscall *HandleEvent)(void *ecx, D3_UI_EVENT_PTR sevt, D3_UI_EVENT_PTR evt); //0x20
		void *_24[17];
		void (__thiscall *SetRect)(void *ecx, float*, float*);//0x68 <pos, sz> ?? forget, not sure
		void (__thiscall *SetRegion)(void *ecx, float*, float*);//0x6C ??
		void *_70[1];
		void (__thiscall *GetTips)(void *ecx); //0x74??
		void *_78[3];
		void (__thiscall *SetText)(void *ecx, char*, int); //0x84
		void *_88[3];
		void (__thiscall *Hover)(void *ecx, int, int); //0x94
	} *vftb;
	void *_04[9];
	unsigned int is_visible;//0x28
	void *_2C;
	D3_UI_INFO info; //0x30
	void *_238[131];
	void *ctx; //0x444; point to container?
	void *_448[48];
	D3_FLOAT_POINT_4 pos0; //0x508 <p0_x, p0_y, p1_x, p1_y> relate to main ? ref<1600, 1200>
	D3_FLOAT_POINT_4 pos1; //0x518 <p0_x, p0_y, p1_x, p1_y> relate to ??
	D3_FLOAT_POINT_4 pos2; //0x528 <x, y, w, h> relate to container?
	void *_538[7];
	void (__cdecl *onclick)(D3_UI_INFO_PTR ui_info); //0x554
	void *_558[354];
	union {
		char *text; //0xAE0
		struct {
			int min_idx;
			int max_idx;
			int cur_idx;
		} sb;
		struct {
			unsigned int start;
			unsigned int end;
		} timer;
	}; //0xAE0
	void *_AEC[88];
	char *_text; //0xC4C
	void *_C50[7];
	unsigned long flag; //0xC6C char
	void *_C70[6];
	struct {
		void *gfx; //?
		int idx;
	} icon[3]; //0xC88

} D3_UI, *D3_UI_PTR;

#define D3_UI_SB_ScrollTo D3__FUNC(void, __thiscall, 0xB3E810, (void *ui_sb, int idx, int update))
// combobox select 0x9954B0(index, click, update)
// set text
#define D3_UI_CB_SetSelectionIdx D3__FUNC(void, __thiscall, 0x9954B0, (void *ui_cb, int idx, int click, int update))
#define D3_UI_COMBOBOX_VFTB 0x014810F0
typedef struct
{
	struct {
	} *vftb;
	void *_04[9];
	unsigned int is_visible;//0x28
	void *_2C;
	D3_UI_INFO info; //0x30
	void *_238[558];
	D3_UI_PTR inner_ui; //0xAF0
	void *_AF4[262];
	int cur_idx; //0xF0C
	struct LIST_CONTENT {
		D3_STRING s;
		void *_0C[2];
	} *str_lst; //0xF10
	void *_F14;
	int str_lst_sz; //0xF18

} D3_UI_COMBOBOX, *D3_UI_COMBOBOX_PTR;

typedef struct
{
	unsigned long long low;
	unsigned long long high;
} D3_ITEM_ID, *D3_ITEM_ID_PTR;

//create_auction
//selected -> 0xC62F70 or 0xCC0550
//Bnet_AuctionHouse:ErrorExceededClaimabl
//Bnet_AuctionHouse:ClaimableItemLimitDia
typedef struct
{
	void *vftb;
	void *other_fields;
	int cache_size;
	D3_ITEM_ID_PTR item_id;
	void *generator; //0x10
	unsigned int bits;
	int item_slot;
	int square_index;
} D3_BASE_ITEM, *D3_BASE_ITEM_PTR;
typedef struct : D3_BASE_ITEM
{
	char name[16]; //0x20
} D3_BASE_ITEM_EX, *D3_BASE_ITEM_EX_PTR;

typedef D3_BASE_ITEM D3_AH_INV_ITEM, *D3_AH_INV_ITEM_PTR;
typedef struct
{
	struct {
		unsigned long long low;
		unsigned long long high;
	} id;
	D3_SIMPLE_LIST inv_lst;
	void *_3C[5];
} D3_AH_INVENTORY_HEROINV, *D3_AH_INVENTORY_HEROINV_PTR;
typedef struct
{
	void *_00[30];
	D3_AH_INVENTORY_HEROINV stash_inv; //0x78
	struct HEROINV_LIST {
		D3_AH_INVENTORY_HEROINV inv;
		HEROINV_LIST *prev;//0x50 ?
		HEROINV_LIST *next;//0x54
	} *hero_inv_lst; //0xC8
} D3_AH_INVENTORY, *D3_AH_INVENTORY_PTR;

typedef struct 
{
	void *_00[151];
	struct {
		void *_00[2];
		struct {
			void *_00[4 + 4 + 4 + 2];
			struct {
				unsigned long long low;
				unsigned long long high;
			} id;
		} *cur_hero;
	} *hero;
} D3_AH_HERO, *D3_AH_HERO_PTR;
#define D3_AH_ClickInventoryItem D3__FUNC(void, __thiscall, 0x0C78170, (void *hero, unsigned long long id_low, unsigned long long id_high, void *item))

typedef struct
{
	struct {
		void *_00;
		D3_AH_HERO_PTR hero; //0x04
		void *_08;
		D3_AH_INVENTORY_PTR inv; //0x0C
	} *ah;
} D3_AUCTION_HOUSE, *D3_AUCTION_HOUSE_PTR;
#define D3_GetAuctionHouse() D3__VAR_DEREF(D3_AUCTION_HOUSE_PTR, 0x180423C)

//D3_ITEM - protobuf size: 0x108 Bnet_AuctionHouse:Sold
typedef struct : D3_BASE_ITEM_EX
{
	void *_30[36];
	unsigned long long curbid; //0xC0
	unsigned long long buyout; //0xC8
	unsigned long long maxbid; //0xD0
	unsigned long flag; //0xD8
	void *_DC;
	void *_E0[4];
	unsigned long long end_ts; //0xF0
	void *_F8[4];
} D3_SEARCH_ITEM, *D3_SEARCH_ITEM_PTR; //0x108

typedef struct : D3_BASE_ITEM_EX
{
	void *_30[26];
	unsigned long long qty; //0x98 -> total = qty * buyout
	unsigned long long start_ts; //0xA0
	unsigned long long end_ts; //0xA8
	void *_B0[10];
	unsigned long long buyout; //0xD8
	unsigned long long minbid; //0xE0
	unsigned long long maxbid; //0xE8
	void *_F0[24];
	unsigned int state; //0x150
	void *_154[59];
	unsigned long long curbid; //0x240
} D3_SELL_ITEM, *D3_SELL_ITEM_PTR; //0x248

typedef struct : D3_BASE_ITEM_EX
{
	void *_30[30];
	unsigned long long end_ts; //0xA8
	void *_B0[6];
	unsigned long long minbid; //0xC8
	void *_D0[4];
	unsigned long long mybid; //0xE0
	void *_E8[84];
	unsigned long long buyout; //0x238
	unsigned long long maxbid; //0x240
	unsigned long long curbid; //0x248
	void *_250[2];
} D3_BID_ITEM, *D3_BID_ITEM_PTR; //0x258

typedef struct
{
	unsigned int idx;
	D3_UI_ITEM_LIST_CONTAINER_PTR cont;
} D3_ITEM_CONTAINER_LIST_NODE, *D3_ITEM_CONTAINER_LIST_NODE_PTR;
#define D3_GetItemContainerList() D3__VAR(D3_SIMPLE_LIST_PTR, 0x16A8FB0) //auction house list_item_container

#define D3_ACD_HID_ARRAY_SIZE (0x40F0 / 4)
typedef struct
{
	void *_00[46];
	void *item; //0xB8
	void *_BC[5];
	struct {
		void *_00[28];
		D3_OBJECT_MANAGER_PTR fast_attr_grp_mgr; //rel:0x70
	} *attr_store; //0xD0
	void *_D4[2];
	struct {
		D3_OBJECT_MANAGER_PTR acd_mgr;
		unsigned long *acd_hid_array; //0x40F0 Byte
	} *acd_store; //0xDC
	void *_E0[23];
	D3_OBJECT_MANAGER_PTR actor_mgr; //abs:0x8C8
	void *_8CC[28];
	struct {
		D3_LIST_MANAGER_PTR ui_mgr;
		void *_04;
		D3_UI_INFO cur_ui;
		void *_210[520];
		D3_UI_INFO ui_onclick; //0xA30
	} *ui_store; //0x93C

	//0x950 cursor_item?
} D3_TLS, *D3_TLS_PTR;

typedef struct
{
	void *_00[483];
	D3_TLS tls; //0x78C
} D3_GLOBAL, *D3_GLOBAL_PTR;


//-----------------------------

typedef struct
{
	unsigned int sz;
	const char *name;
} D3_PACKET_TYPE, *D3_PACKET_TYPE_PTR;

D3_PACKET_TYPE D3_ServerPacketTypeTable[];

#pragma pack(pop)

#endif
