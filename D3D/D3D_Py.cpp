#include "D3D.h"
#include <python.h>
#include <structmember.h>
#include <Windows.h>

typedef enum
{
	LKID_NONE = 0,
	LKID_UI,
	LKID_UI_INFO,
	LKID_SLOT,
	LKID_ACD,
	LKID_ACTOR,
	LKID_AH_SEARCH_ITEMLIST,
	LKID_AH_SEARCH_ITEM,
	LKID_AH_OTHER_ITEMLIST,
	LKID_AH_SELL_ITEM,
	LKID_AH_BID_ITEM,
	LKID_AH_INVENTORY,
	LKID_AH_INVENTORY_ITEM,
	LKID_POINT4,
	LKID_END
} D3Py_LOOKUP_ID;

typedef struct
{
	char *name;
	unsigned int type;
	union {
		unsigned int offset;
		PyObject* (*method)(PyObject*, PyObject*, PyObject*);
	};
	unsigned int lkid;
	unsigned int offset_ref;
} D3Py_LOOKUP_ATTR_DEF, *D3Py_LOOKUP_ATTR_DEF_PTR;

static struct
{
	PyObject* lookup;
	D3Py_LOOKUP_ATTR_DEF_PTR attr_def;
} g_lookup[ D3Py_LOOKUP_ID::LKID_END ];

#define D3Py_INIT_LOOKUP_ATTR_DEF(LKID) \
static struct __init_lookup_##LKID { \
	__init_lookup_##LKID() \
	{ \
		extern D3Py_LOOKUP_ATTR_DEF __lookup_##LKID[]; \
		g_lookup[ D3Py_LOOKUP_ID::##LKID ].attr_def = __lookup_##LKID; \
	} \
} ____init_lookup_##LKID; \
D3Py_LOOKUP_ATTR_DEF __lookup_##LKID[]

#define D3Py_INTERNAL_VAL_SIZE 3
struct D3Py_INTERNAL
{
	PyObject_HEAD
	PyObject *dep;
	void *c_this;
	union {
		PyObject* (*next)(PyObject*);
		PyObject *attrs;
		PyObject* (*method)(PyObject*, PyObject*, PyObject*);
	};
	union {
		unsigned int uint_v;
		int int_v;
		void *ptr_v;
	} val[D3Py_INTERNAL_VAL_SIZE];
};
typedef D3Py_INTERNAL D3Py_ITERATOR, *D3Py_ITERATOR_PTR;
typedef D3Py_INTERNAL D3Py_OBJECT, *D3Py_OBJECT_PTR;
typedef D3Py_INTERNAL D3Py_OBJECT_METHOD, *D3Py_OBJECT_METHOD_PTR;

static PyTypeObject D3Py_Iterator_Type = {
PyObject_HEAD_INIT(NULL)
0,                         /*ob_size*/
"D3_Iterator",             /*tp_name*/
sizeof(D3Py_ITERATOR), /*tp_basicsize*/
};

static PyTypeObject D3Py_Object_Type = {
PyObject_HEAD_INIT(NULL)
0,                         /*ob_size*/
"D3_Object",             /*tp_name*/
sizeof(D3Py_OBJECT), /*tp_basicsize*/
};

static PyTypeObject D3Py_Object_Method_Type = {
PyObject_HEAD_INIT(NULL)
0,                         /*ob_size*/
"D3_Object_Method",             /*tp_name*/
sizeof(D3Py_OBJECT_METHOD), /*tp_basicsize*/
};

#define T_D3PY_OBJECT_METHOD 0xF0FFFFFF
#define T_D3PY_OBJECT 0xF0FFFFFE
#define T_D3PY_OBJECT_INPLACE 0xF0FFFFFD


static PyObject* alloc_pyobj(PyTypeObject *type, int lkid=-1, PyObject *dep=0)
{
	D3Py_INTERNAL *o = PyObject_New(D3Py_INTERNAL, type);
	if(!dep || !((D3Py_INTERNAL*)dep)->dep) { o->dep = 0; } else { Py_INCREF(dep); o->dep = dep; }
	o->c_this = 0;
	o->attrs = lkid >= 0 ? g_lookup[lkid].lookup : 0;

	return (PyObject*)o;
}

static PyObject* D3Py_Sleep(PyObject *self, PyObject *args)
{
	unsigned int ms = 0;
	if( !PyArg_ParseTuple(args, "|I", &ms) ) return 0;

	extern unsigned int worker_sleep(unsigned int ms);
	return PyLong_FromUnsignedLong(worker_sleep(ms));
}

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_POINT4) = {
	{"v0", T_FLOAT, offsetof(D3_FLOAT_POINT_4, i)},
	{"v1", T_FLOAT, offsetof(D3_FLOAT_POINT_4, j)},
	{"v2", T_FLOAT, offsetof(D3_FLOAT_POINT_4, k)},
	{"v3", T_FLOAT, offsetof(D3_FLOAT_POINT_4, z)},
    {0}
};


static PyObject* ui_get_flag(PyObject *self, PyObject *args, PyObject *kws)
{
	D3_UI_PTR ui = (D3_UI_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	if((unsigned long)ui->vftb != D3_UI_COMBOBOX_VFTB) return PyLong_FromUnsignedLong(ui->flag);
	Py_RETURN_NONE;
}

static PyObject* ui_click(PyObject *self, PyObject *args, PyObject *kws)
{
	D3_UI_PTR ui = (D3_UI_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	if((unsigned long)ui->vftb != D3_UI_COMBOBOX_VFTB && ui->onclick) ui->onclick(&ui->info);
	Py_RETURN_NONE;
}

static PyObject* ui_send_click(PyObject *self, PyObject *args, PyObject *kws)
{
	D3_UI_PTR ui = (D3_UI_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	if((unsigned long)ui->vftb == D3_UI_COMBOBOX_VFTB) Py_RETURN_NONE;

	D3_GetTLS()->ui_store->ui_onclick.ui_hid = ui->info.ui_hid;

	D3_UI_EVENT sevt, evt;

	memset(&sevt, 0x00, sizeof(sevt));
	D3_UI_InitEvent(&sevt, 0);
	sevt.evt_id = 1;
	memcpy(&evt, &sevt, sizeof(sevt));
	evt.evt_id = D3_UI_EVENT_MOUSE_DOWN;
	ui->vftb->HandleEvent(ui, &sevt, &evt);

	memset(&sevt, 0x00, sizeof(sevt));
	D3_UI_InitEvent(&sevt, 0);
	sevt.evt_id = 1;
	memcpy(&evt, &sevt, sizeof(sevt));
	evt.evt_id = D3_UI_EVENT_MOUSE_UP;
	ui->vftb->HandleEvent(ui, &sevt, &evt);

	Py_RETURN_NONE;
}

static PyObject* ui_set_text(PyObject *self, PyObject *args, PyObject *kws)
{
	char *s;
	int flag = 0;
	if( !PyArg_ParseTuple(args, "s|i", &s, &flag) ) return 0;

	D3_UI_PTR ui = (D3_UI_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	if( (unsigned long)ui->vftb != D3_UI_COMBOBOX_VFTB ) {
		ui->vftb->SetText(ui, s, flag);

	} else {
		D3_UI_COMBOBOX_PTR cb_ui = (D3_UI_COMBOBOX_PTR)ui;

		int sz = cb_ui->str_lst_sz;
		int i = 0;
		D3_UI_COMBOBOX::LIST_CONTENT *lst = cb_ui->str_lst;
		for(; i < sz; i++) {
			char *t = lst[i].s.s;
			if(t && !strcmp(t, s)) {
				D3_UI_CB_SetSelectionIdx(cb_ui, i, 0, 1);
				break;
			}
		}
		if(i >= sz) D3_UI_CB_SetSelectionIdx(cb_ui, -1, 0, 1);

	}
	Py_RETURN_NONE;
}

static PyObject* ui_get_text(PyObject *self, PyObject *args, PyObject *kws)
{
	int flag = 0;
	if( !PyArg_ParseTuple(args, "|i", &flag) ) return 0;

	D3_UI_PTR ui = (D3_UI_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	if( (unsigned long)ui->vftb != D3_UI_COMBOBOX_VFTB ) {
		if(ui->text) return PyString_FromString(ui->text);

	} else {
		D3_UI_COMBOBOX_PTR cb_ui = (D3_UI_COMBOBOX_PTR)ui;
		if(!flag) {
			if(cb_ui->cur_idx >= 0 && cb_ui->cur_idx < cb_ui->str_lst_sz) {
				char *s = cb_ui->str_lst[ cb_ui->cur_idx ].s.s;
				if(s) return PyString_FromString(s);
			}
		} else {
			PyObject *l = PyList_New(0);
			int sz = cb_ui->str_lst_sz;
			int i = 0;
			D3_UI_COMBOBOX::LIST_CONTENT *lst = cb_ui->str_lst;
			for(; i < sz; i++) {
				char *t = lst[i].s.s;
				if(!t) continue;
				PyObject *o = PyString_FromString(t);
				PyList_Append(l, (PyObject*)o);
				Py_DECREF(o);
			}
			return l;
		}
	}

	Py_RETURN_NONE;
}

static PyObject* ui_set_region(PyObject *self, PyObject *args, PyObject *kws)
{
	float a[4];
	if( !PyArg_ParseTuple(args, "ffff", &a[0], &a[1], &a[2], &a[3]) ) return 0;

	D3_UI_PTR ui = (D3_UI_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	if((unsigned long)ui->vftb != D3_UI_COMBOBOX_VFTB) ui->vftb->SetRegion(ui, a, a + 2);
	Py_RETURN_NONE;
}

static PyObject* ui_set_rect(PyObject *self, PyObject *args, PyObject *kws)
{
	float a[4];
	if( !PyArg_ParseTuple(args, "ffff", &a[0], &a[1], &a[2], &a[3]) ) return 0;

	D3_UI_PTR ui = (D3_UI_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	if((unsigned long)ui->vftb != D3_UI_COMBOBOX_VFTB) ui->vftb->SetRect(ui, a, a + 2);
	Py_RETURN_NONE;
}

static PyObject* ui_scroll_to(PyObject *self, PyObject *args, PyObject *kws)
{
	int idx = 0;
	if( !PyArg_ParseTuple(args, "i", &idx) ) return 0;

	D3_UI_PTR ui = (D3_UI_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	D3_UI_SB_ScrollTo(ui, idx, 1);
	Py_RETURN_NONE;
}


D3Py_INIT_LOOKUP_ATTR_DEF(LKID_UI) = {
    {"hid", T_ULONGLONG, offsetof(D3_UI, info.ui_hid) },
    {"name", T_STRING_INPLACE, offsetof(D3_UI, info.name) },
	{"text", T_STRING, offsetof(D3_UI, text) },
	{"visible", T_UINT, offsetof(D3_UI, is_visible) },

	//scroll bar only
	{"cur_idx", T_UINT, offsetof(D3_UI, sb.cur_idx) },
	{"min_idx", T_UINT, offsetof(D3_UI, sb.min_idx) },
	{"max_idx", T_UINT, offsetof(D3_UI, sb.max_idx) },
	{"ScrollTo", T_D3PY_OBJECT_METHOD, (unsigned int)ui_scroll_to },
	//

	//button text
	{"GetFlag", T_D3PY_OBJECT_METHOD, (unsigned int)ui_get_flag },

	{"SendClick", T_D3PY_OBJECT_METHOD, (unsigned int)ui_send_click },
	{"Click", T_D3PY_OBJECT_METHOD, (unsigned int)ui_click },
	{"SetText", T_D3PY_OBJECT_METHOD, (unsigned int)ui_set_text },
	{"GetText", T_D3PY_OBJECT_METHOD, (unsigned int)ui_get_text },
	{"SetRegion", T_D3PY_OBJECT_METHOD, (unsigned int)ui_set_region },
	{"SetRect", T_D3PY_OBJECT_METHOD, (unsigned int)ui_set_rect },
	
    {0}
};

static PyObject* ui_iter_next(PyObject *self)
{
	D3Py_ITERATOR_PTR s = (D3Py_ITERATOR_PTR)self;

	D3_LIST_MANAGER_PTR mgr = (D3_LIST_MANAGER_PTR)s->c_this;
	D3_LIST_PTR *lst_arr = mgr->list_array;
	int idx = s->val[0].int_v;
	D3_LIST_PTR lst = (D3_LIST_PTR)s->val[1].ptr_v;
	
	if(idx >= mgr->list_array_size) return 0;
	if(idx < 0) {
		idx = 0;
		lst = lst_arr[0];
	}

	D3Py_OBJECT_PTR o = 0;
	while(lst || ++idx < mgr->list_array_size) {
		if(lst) {
			void *ptr = *(void**)((char*)&lst->data + sizeof(D3_UI_INFO));
			lst = lst->next;
			if(ptr) {
				o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_UI, self);
				o->c_this = ptr;
				break;
			}
		} else
			lst = lst_arr[idx];
	}

	s->val[0].int_v = idx;
	s->val[1].ptr_v = lst;

	return (PyObject*)o;
}

static PyObject* D3Py_GetUI(PyObject *self, PyObject *args)
{
	unsigned long long uid_hid = -1;
	if( !PyArg_ParseTuple(args, "|K", &uid_hid) ) return 0;

	if( PyTuple_Size(args) > 0 ) {
		D3_UI_PTR ui = D3_HasUI(&uid_hid) ? D3_GetUI(&uid_hid) : 0;
		if(!ui) Py_RETURN_NONE;

		D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_UI);
		o->c_this = ui;

		return (PyObject*)o;

	} else {
		D3Py_ITERATOR_PTR o = (D3Py_ITERATOR_PTR)alloc_pyobj(&D3Py_Iterator_Type);
		o->c_this = D3_GetTLS()->ui_store->ui_mgr;
		o->next = ui_iter_next;
		o->val[0].int_v = -1;

		return (PyObject*)o;

	}

	Py_RETURN_NONE;
}

static PyObject *acd_get_gold(PyObject *self, PyObject *args, PyObject *kws)
{
	D3_ACD_PTR acd = (D3_ACD_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	return PyLong_FromUnsignedLongLong( D3_GetGold(acd) );
}

static PyObject *acd_get_attr(PyObject *self, PyObject *args, PyObject *kws)
{
	int ret_type = 0;
	unsigned long attr_id;
	if( !PyArg_ParseTuple(args, "k|i", &attr_id, &ret_type) ) return 0;

	D3_ACD_PTR acd = (D3_ACD_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	if(ret_type == 0)
		return PyFloat_FromDouble( D3_GetAttr_AsFloat(acd, attr_id) );
	else if(ret_type == 1)
		return PyLong_FromLong( D3_GetAttr_AsInt(acd, attr_id) );
	else
		return PyLong_FromUnsignedLong( D3_GetAttr_AsInt(acd, attr_id) );
}

static PyObject *acd_get_item_name(PyObject *self, PyObject *args, PyObject *kws)
{
	int flag0 = 0;
	int flag1 = 0;
	if( !PyArg_ParseTuple(args, "|ii", &flag0, &flag1) ) return 0;

	D3_ACD_PTR acd = (D3_ACD_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	D3_STRING s;
	D3_StringInit(&s);
	D3_GetItemName(&s, acd, flag0, flag1);
	PyObject *ret = PyString_FromString(s.s);
	D3_StringDel(&s);

	return ret;
}


static PyObject *slot_get_item_acd(PyObject *self, PyObject *args, PyObject *kws)
{
	unsigned long x = 0;
	unsigned long y = 0;
	if( !PyArg_ParseTuple(args, "kk", &x, &y) ) return 0;

	D3Py_OBJECT_PTR s = (D3Py_OBJECT_PTR)self;
	if(x < 0 || y < 0 || x >= s->val[0].uint_v || y >= s->val[1].uint_v) Py_RETURN_NONE;
	unsigned long *inv = (unsigned long *)s->c_this;
	unsigned long acd_hid = inv[  y * s->val[2].uint_v + x ];
	
	D3_ACD_PTR acd = D3_GetACD(acd_hid);
	if(!acd) Py_RETURN_NONE;

	D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_ACD, self);
	o->c_this = acd;

	return (PyObject*)o;
}

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_SLOT) = {
	{"slot", T_ULONG, offsetof(D3Py_OBJECT, val[2]), 0, -1 },
	{"w", T_ULONG, offsetof(D3Py_OBJECT, val[0]), 0, -1 },
	{"h", T_ULONG, offsetof(D3Py_OBJECT, val[1]), 0, -1 },
	{"GetItemACD", T_D3PY_OBJECT_METHOD, (unsigned int)slot_get_item_acd },
    {0}
};

static PyObject *acd_get_slot(PyObject *self, PyObject *args, PyObject *kws)
{
	unsigned long slot_id = 0;
	if( !PyArg_ParseTuple(args, "k", &slot_id) ) return 0;

	D3_ACD_PTR acd = (D3_ACD_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	unsigned long t[2] = {0, 0};
	unsigned long *m = D3_GetSquare(acd, slot_id, (unsigned long*)&t);
	if(!m) Py_RETURN_NONE;

	D3_GetSlotSize(slot_id, (unsigned long*)&t);

	D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_SLOT, self);
	o->c_this = m;
	o->val[0].uint_v = t[0];
	o->val[1].uint_v = t[1];
	o->val[2].uint_v = slot_id;

	return (PyObject*)o;
}

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_ACD) = {
    {"hid", T_ULONG, offsetof(D3_ACD, acd_hid) },
    {"name", T_STRING_INPLACE, offsetof(D3_ACD, name) },
	{"actor_hid", T_ULONG, offsetof(D3_ACD, actor_hid) },
	{"bnet_hid", T_ULONG, offsetof(D3_ACD, bnet_hid) },
	{"attr_hid", T_ULONG, offsetof(D3_ACD, attr_hid) },

	{"type", T_ULONG, offsetof(D3_ACD, type) },

	{"pos0", T_D3PY_OBJECT_INPLACE, offsetof(D3_ACD, pt0), LKID_POINT4 },
	{"pos1", T_D3PY_OBJECT_INPLACE, offsetof(D3_ACD, pt1), LKID_POINT4 },
	{"pos2", T_D3PY_OBJECT_INPLACE, offsetof(D3_ACD, pt2), LKID_POINT4 },

	{"GetGold", T_D3PY_OBJECT_METHOD, (unsigned int)acd_get_gold },
	{"GetAttr", T_D3PY_OBJECT_METHOD, (unsigned int)acd_get_attr },
	{"GetItemName", T_D3PY_OBJECT_METHOD, (unsigned int)acd_get_item_name },
	{"GetSlot", T_D3PY_OBJECT_METHOD, (unsigned int)acd_get_slot },
    {0}
};

static PyObject* D3Py_GetCurACD(PyObject *self, PyObject *args)
{
	unsigned long hid = D3_GetCurACDHid();
	D3_ACD_PTR acd = D3_GetACD(hid);
	if(!acd) Py_RETURN_NONE;
	D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_ACD);
	o->c_this = acd;
	return (PyObject*)o;
}

static PyObject* acd_iter_next(PyObject *self)
{
	D3Py_ITERATOR_PTR s = (D3Py_ITERATOR_PTR)self;
	unsigned long *hid = (unsigned long *)s->c_this;
	unsigned long m = s->val[0].uint_v;
	unsigned long i = s->val[1].uint_v;
	unsigned long c = s->val[2].uint_v;
	D3Py_OBJECT_PTR o = 0;
	for(; c < m && i < D3_ACD_HID_ARRAY_SIZE - 1; i++) {
		if(hid[i] == -1) continue;
		D3_ACD_PTR acd = D3_GetACD(hid[i]);
		if(!acd) continue;
		o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_ACD, self);
		o->c_this = acd;
		c++;
		i++;
		break;
	}
	s->val[1].uint_v = i;
	s->val[2].uint_v = c;
	return (PyObject*)o;
}

static PyObject* D3Py_GetACD(PyObject *self, PyObject *args)
{
	unsigned long acd_hid = -1;
	if( !PyArg_ParseTuple(args, "|k", &acd_hid) ) return 0;

	if( PyTuple_Size(args) > 0 ) {
		D3_ACD_PTR acd = D3_GetACD(acd_hid);
		if(!acd) Py_RETURN_NONE;

		D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_ACD);
		o->c_this = acd;
		return (PyObject*)o;

	} else {
		D3_OBJECT_MANAGER_PTR mgr = D3_GetTLS()->acd_store ? D3_GetTLS()->acd_store->acd_mgr : 0;
		if(mgr) {
			D3Py_ITERATOR_PTR o = (D3Py_ITERATOR_PTR)alloc_pyobj(&D3Py_Iterator_Type);
			o->c_this = D3_GetTLS()->acd_store->acd_hid_array;
			o->next = acd_iter_next;
			o->val[0].uint_v = mgr->count;
			o->val[1].uint_v = 0;
			o->val[2].uint_v = 0;
			return (PyObject*)o;
		} else
			return PyTuple_New(0);
	}

	Py_RETURN_NONE;
}

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_ACTOR) = {
    {"hid", T_ULONG, offsetof(D3_ACTOR, actor_hid) },
	{"acd_hid", T_ULONG, offsetof(D3_ACTOR, acd_hid) },
	{"name", T_STRING_INPLACE, offsetof(D3_ACTOR, name) },
    {0}
};
static PyObject* D3Py_GetActor(PyObject *self, PyObject *args)
{
	unsigned long acd_hid = -1;
	if( !PyArg_ParseTuple(args, "k", &acd_hid) ) return 0;

	D3_ACTOR_PTR actor = D3_GetActor(acd_hid);
	if(!actor) Py_RETURN_NONE;

	D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_ACTOR);
	o->c_this = actor;
	return (PyObject*)o;
}

void cap_dealloc(PyObject *o)
{
	D3_ACD_PTR acd = (D3_ACD_PTR)PyCapsule_GetPointer(o, 0);
	//printf("killed acd %08X \n", acd);
	D3_DeleteACD(acd->acd_hid);
}
static PyObject* ah_item_get_acd(PyObject *self, PyObject *args, PyObject *kws)
{
	D3_BASE_ITEM_PTR item = (D3_BASE_ITEM_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	D3_ACD_PTR acd = D3_GetACD( D3_CreateACD(D3_ACD_HID_ARRAY_SIZE - 1, item->generator) );
	if(!acd) Py_RETURN_NONE;

	D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_ACD);
	o->dep = PyCapsule_New(acd, 0, cap_dealloc);
	o->c_this = acd;
	return (PyObject*)o;
}

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_AH_SEARCH_ITEM) = {
	{"maxbid", T_ULONGLONG, offsetof(D3_SEARCH_ITEM, maxbid) },
	{"curbid", T_ULONGLONG, offsetof(D3_SEARCH_ITEM, curbid) },
	{"buyout", T_ULONGLONG, offsetof(D3_SEARCH_ITEM, buyout) },
	{"endingts", T_ULONGLONG, offsetof(D3_SEARCH_ITEM, end_ts) },

	{"GetACD", T_D3PY_OBJECT_METHOD, (unsigned int)ah_item_get_acd },

	{"id_low", T_ULONGLONG, offsetof(D3_ITEM_ID, low), 0, 0xFFFF0000 },
	{"id_high", T_ULONGLONG, offsetof(D3_ITEM_ID, high), 0, 0xFFFF0000 },

    {0}
};

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_AH_SELL_ITEM) = {
	{"maxbid", T_ULONGLONG, offsetof(D3_SELL_ITEM, maxbid) },
	{"curbid", T_ULONGLONG, offsetof(D3_SELL_ITEM, curbid) },
	{"minbid", T_ULONGLONG, offsetof(D3_SELL_ITEM, minbid) },
	{"buyout", T_ULONGLONG, offsetof(D3_SELL_ITEM, buyout) },
	{"qty", T_ULONGLONG, offsetof(D3_SELL_ITEM, qty) },
	{"endingts", T_ULONGLONG, offsetof(D3_SELL_ITEM, end_ts) },
	{"state", T_ULONG, offsetof(D3_SELL_ITEM, state) },

	{"GetACD", T_D3PY_OBJECT_METHOD, (unsigned int)ah_item_get_acd },

	{"id_low", T_ULONGLONG, offsetof(D3_ITEM_ID, low), 0, 0xFFFF0000 },
	{"id_high", T_ULONGLONG, offsetof(D3_ITEM_ID, high), 0, 0xFFFF0000 },

    {0}
};

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_AH_BID_ITEM) = {
	{"maxbid", T_ULONGLONG, offsetof(D3_BID_ITEM, maxbid) },
	{"curbid", T_ULONGLONG, offsetof(D3_BID_ITEM, curbid) },
	{"minbid", T_ULONGLONG, offsetof(D3_BID_ITEM, minbid) },
	{"buyout", T_ULONGLONG, offsetof(D3_BID_ITEM, buyout) },
	{"mybid", T_ULONGLONG, offsetof(D3_BID_ITEM, mybid) },
	{"endingts", T_ULONGLONG, offsetof(D3_BID_ITEM, end_ts) },

	{"GetACD", T_D3PY_OBJECT_METHOD, (unsigned int)ah_item_get_acd },

	{"id_low", T_ULONGLONG, offsetof(D3_ITEM_ID, low), 0, 0xFFFF0000 },
	{"id_high", T_ULONGLONG, offsetof(D3_ITEM_ID, high), 0, 0xFFFF0000 },

    {0}
};

static struct ITEMLIST_LK_DEF
{
	unsigned long long hid;
	unsigned long lst_offset;
	int itemlist_lkid;
	int item_lkid;
	int item_size;
} _g_itemlist_lk_def[] = {
	{0xA0002EC36996C1F3, offsetof(D3_UI_ITEM_LIST_CONTAINER::INNER::SEARCH_PAGE_DATA, item_list), LKID_AH_SEARCH_ITEMLIST, LKID_AH_SEARCH_ITEM, sizeof(D3_SEARCH_ITEM) }, //search
	{0xF3AD577D9E07F0AE, offsetof(D3_UI_ITEM_LIST_CONTAINER::INNER::OTHER_DATA, item_list), LKID_AH_OTHER_ITEMLIST, LKID_AH_SELL_ITEM, sizeof(D3_SELL_ITEM) }, //sell
	{0x4EA194AF90C3D9D3, offsetof(D3_UI_ITEM_LIST_CONTAINER::INNER::OTHER_DATA, item_list), LKID_AH_OTHER_ITEMLIST, LKID_AH_BID_ITEM, sizeof(D3_BID_ITEM) }, //bid
//	0x7C8BD37A25E8BAD3 //completed
};

static PyObject *ah_itemlist_get_item(PyObject *self, PyObject *args, PyObject *kws)
{
	int idx = 0;
	if( !PyArg_ParseTuple(args, "i", &idx) ) return 0;
	ITEMLIST_LK_DEF *x = &_g_itemlist_lk_def[ ((D3Py_OBJECT_PTR)self)->val[1].uint_v ];
	D3_SIMPLE_LIST_PTR lst = (D3_SIMPLE_LIST_PTR)((char*)((D3Py_OBJECT_PTR)self)->c_this + x->lst_offset);
	if(idx < 0 || idx >= lst->sz) Py_RETURN_NONE;
	
	D3_BASE_ITEM_PTR item = (D3_BASE_ITEM_PTR)((char*)lst->ptr_ip + idx * x->item_size);
	D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, x->item_lkid, self);
	o->c_this = item;
	o->val[0].ptr_v = item->item_id;
	return (PyObject*)o;
}

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_AH_OTHER_ITEMLIST) = {
	{"total", T_INT, offsetof(D3_UI_ITEM_LIST_CONTAINER::INNER::OTHER_DATA, item_list.sz) },

	{"name", T_STRING_INPLACE, offsetof(D3_UI_INFO, name), 0, 0xFFFF0000 },
	{"hid", T_ULONGLONG, offsetof(D3_UI_INFO, ui_hid), 0, 0xFFFF0000 },

	{"GetItem", T_D3PY_OBJECT_METHOD, (unsigned int)ah_itemlist_get_item },
    {0}
};

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_AH_SEARCH_ITEMLIST) = {
	{"num_per_page", T_INT, offsetof(D3_UI_ITEM_LIST_CONTAINER::INNER::SEARCH_PAGE_DATA, num_per_page) },
	{"total", T_INT, offsetof(D3_UI_ITEM_LIST_CONTAINER::INNER::SEARCH_PAGE_DATA, total_items) },
	{"page_idx", T_INT, offsetof(D3_UI_ITEM_LIST_CONTAINER::INNER::SEARCH_PAGE_DATA, page_idx) },
	{"num_cur_page", T_INT, offsetof(D3_UI_ITEM_LIST_CONTAINER::INNER::SEARCH_PAGE_DATA, item_list.sz) },

	{"name", T_STRING_INPLACE, offsetof(D3_UI_INFO, name), 0, 0xFFFF0000 },
	{"hid", T_ULONGLONG, offsetof(D3_UI_INFO, ui_hid), 0, 0xFFFF0000 },

	{"GetItem", T_D3PY_OBJECT_METHOD, (unsigned int)ah_itemlist_get_item },
    {0}
};

static PyObject* D3Py_AH_GetItemList(PyObject *self, PyObject *args)
{
	unsigned long idx = 0;
	if( !PyArg_ParseTuple(args, "k", &idx) ) return 0;
	if(idx >= sizeof(_g_itemlist_lk_def) / sizeof(_g_itemlist_lk_def[0]) ) Py_RETURN_NONE;

	ITEMLIST_LK_DEF *x = &_g_itemlist_lk_def[idx];
	D3_SIMPLE_LIST_PTR lst = D3_GetItemContainerList();
	for(int i = 0; i < lst->sz; i++) {
		D3_ITEM_CONTAINER_LIST_NODE_PTR n = (D3_ITEM_CONTAINER_LIST_NODE_PTR)lst->ptr[i];
		if(n && n->cont && n->cont->info.ui_hid == x->hid) {
			if(n->cont->data) {
				D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, x->itemlist_lkid);
				o->c_this = n->cont->data;
				o->val[0].ptr_v = &n->cont->info;
				o->val[1].uint_v = idx;
				return (PyObject*)o;
			}
			break;
		}
	}
	Py_RETURN_NONE;
}

static PyObject *ah_item_click(PyObject *self, PyObject *args, PyObject *kws)
{
	D3_AH_INV_ITEM_PTR item = (D3_AH_INV_ITEM_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	D3_AH_INVENTORY_HEROINV_PTR hinv = (D3_AH_INVENTORY_HEROINV_PTR)((D3Py_OBJECT_PTR)self)->val[0].ptr_v;

	D3_AUCTION_HOUSE_PTR ah = D3_GetAuctionHouse();
	if(!ah || !ah->ah || !ah->ah->hero) Py_RETURN_NONE;
	D3_AH_HERO_PTR hero = ah->ah->hero;
	if(!hero->hero || !hero->hero->cur_hero) Py_RETURN_NONE;

	if(hinv->id.low || hinv->id.high)
		D3_AH_ClickInventoryItem(hero->hero, hinv->id.low, hinv->id.high, item);
	else
		D3_AH_ClickInventoryItem(hero->hero, hero->hero->cur_hero->id.low, hero->hero->cur_hero->id.high, item);

	Py_RETURN_NONE;
}
D3Py_INIT_LOOKUP_ATTR_DEF(LKID_AH_INVENTORY_ITEM) = {
	{"slot", T_INT, offsetof(D3_AH_INV_ITEM, item_slot) },
	{"idx", T_INT, offsetof(D3_AH_INV_ITEM, square_index) },
	{"GetACD", T_D3PY_OBJECT_METHOD, (unsigned int)ah_item_get_acd },
	{"Click", T_D3PY_OBJECT_METHOD, (unsigned int)ah_item_click },

	{"id_low", T_ULONGLONG, offsetof(D3_ITEM_ID, low), 0, 0xFFFF0001 },
	{"id_high", T_ULONGLONG, offsetof(D3_ITEM_ID, high), 0, 0xFFFF0001 },

    {0}
};
static PyObject *ah_inv_get_item(PyObject *self, PyObject *args, PyObject *kws)
{
	int item_slot = 0;
	int square_index = 0;
	if( !PyArg_ParseTuple(args, "|ii", &item_slot, &square_index) ) return 0;

	D3_AH_INVENTORY_HEROINV_PTR hinv = (D3_AH_INVENTORY_HEROINV_PTR)((D3Py_OBJECT_PTR)self)->c_this;
	D3_AH_INV_ITEM_PTR ptr = (D3_AH_INV_ITEM_PTR)hinv->inv_lst.ptr_ip;
	int sz = hinv->inv_lst.sz;
	if( PyTuple_Size(args) > 0 ) {
		for(int i = 0; i < sz; i++) {
			if(ptr[i].item_slot != item_slot || ptr[i].square_index != square_index) continue;
			D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_AH_INVENTORY_ITEM, self);
			o->c_this = &ptr[i];
			o->val[0].ptr_v = hinv;
			o->val[1].ptr_v = ptr[i].item_id;
			return (PyObject*)o;
		}
	} else {
		PyObject *l = PyList_New(0);
		for(int i = 0; i < sz; i++) {
			D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_AH_INVENTORY_ITEM, self);
			o->c_this = &ptr[i];
			o->val[0].ptr_v = hinv;
			o->val[1].ptr_v = ptr[i].item_id;
			PyList_Append(l, (PyObject*)o);
			Py_DECREF(o);
		}
		return (PyObject*)l;
	}

	Py_RETURN_NONE;
}

D3Py_INIT_LOOKUP_ATTR_DEF(LKID_AH_INVENTORY) = {
	{"id_low", T_ULONGLONG, offsetof(D3_AH_INVENTORY_HEROINV, id.low) },
	{"id_high", T_ULONGLONG, offsetof(D3_AH_INVENTORY_HEROINV, id.high) },
	{"total", T_INT, offsetof(D3_AH_INVENTORY_HEROINV, inv_lst.sz) },
	{"GetItem", T_D3PY_OBJECT_METHOD, (unsigned int)ah_inv_get_item },
    {0}
};
static PyObject* D3Py_AH_GetInventory(PyObject *self, PyObject *args)
{
	unsigned long long id_low = 0;
	unsigned long long id_high = 0;
	if( !PyArg_ParseTuple(args, "|KK", &id_low, &id_high) ) return 0;

	D3_AUCTION_HOUSE_PTR ah = D3_GetAuctionHouse();
	if(!ah || !ah->ah || !ah->ah->inv) Py_RETURN_NONE;
	D3_AH_INVENTORY_PTR inv = ah->ah->inv;

	if( PyTuple_Size(args) > 0 ) {
		D3_AH_INVENTORY_HEROINV_PTR hinv = 0;
		if(!id_low && !id_high) {
			hinv = &inv->stash_inv;
		} else {
			D3_AH_INVENTORY::HEROINV_LIST *hinv_lst = inv->hero_inv_lst;
			while(hinv_lst) {
				if(hinv_lst->inv.id.low == id_low && hinv_lst->inv.id.high == id_high) {
					hinv = &hinv_lst->inv;
					break;
				}
				hinv_lst = hinv_lst->next;
			}
		}
		if(!hinv) Py_RETURN_NONE;
		D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_AH_INVENTORY);
		o->c_this = hinv;
		return (PyObject*)o;

	} else {
		PyObject *l = PyList_New(0);
		D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_AH_INVENTORY);
		o->c_this = &inv->stash_inv;
		PyList_Append(l, (PyObject*)o);
		Py_DECREF(o);

		D3_AH_INVENTORY::HEROINV_LIST *hinv_lst = inv->hero_inv_lst;
		while(hinv_lst) {
			o = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, LKID_AH_INVENTORY);
			o->c_this = &hinv_lst->inv;
			PyList_Append(l, (PyObject*)o);
			Py_DECREF(o);
			hinv_lst = hinv_lst->next;
		}

		return (PyObject*)l;
	}

	Py_RETURN_NONE;
}

static PyObject* D3Py_CallCFunction(PyObject *self, PyObject *args)
{
	/*
	void *func_ptr;
	if( !PyArg_ParseTuple(args, "k", &func_ptr) ) return 0;

	D3_AUCTION_HOUSE_PTR ah = D3_GetAuctionHouse();
	if(!ah || !ah->ah) Py_RETURN_NONE;
	*/
	void screen2gamepos();screen2gamepos();

	Py_RETURN_NONE;
}

static PyObject* D3Py_ReadMemory(PyObject *self, PyObject *args)
{
	unsigned long ptr;
	unsigned long sz;
	if( !PyArg_ParseTuple(args, "kk", &ptr, &sz) ) return 0;

	char _buf[1024];
	char *buf = _buf;
	if(sz > sizeof(_buf)) buf = (char *)malloc(sz);

	__try {
		memcpy(buf, (void*)ptr, sz);
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		sz = 0;
	}

	PyObject *ret = sz ? PyString_FromStringAndSize(buf, sz) : 0;
	if(buf != _buf) free(buf);
	if(ret) { return ret; } else { Py_RETURN_NONE; }
}

static PyObject* D3Py_GetTickCount(PyObject *self, PyObject *args)
{
	return PyLong_FromUnsignedLong(GetTickCount());
}

static PyObject *D3Py_UseSkill(PyObject *self, PyObject *args)
{
	unsigned int actor_id = -1;
	unsigned int skill_id = 0;
	unsigned int x = 0;
	unsigned int y = 0;
	if( !PyArg_ParseTuple(args, "kkk|k", &skill_id, &x, &y, &actor_id) ) return 0;

	D3_ACD_PTR acd = D3_GetACD( D3_GetCurACDHid() );
	if(!acd) Py_RETURN_NONE;
	D3_ACTOR_PTR actor = D3_GetActor(acd->actor_hid);
	if(!actor) Py_RETURN_NONE;

	D3_GAME_DATA_PTR gd = D3_GameData();
	gd->x = x;
	gd->y = y;
	
	D3_USESKILL_DATA ud = {actor_id, skill_id, skill_id, 0, -1};
	if(actor_id == -1) {

	} else {

	}

	return PyInt_FromLong(D3_UseSkill(actor, &ud, 1, 0, 0));
}

static PyObject *D3Py_Pos_WorldToScreen(PyObject *self, PyObject *args)
{
	D3_FLOAT_POINT_4 wp4 = {0, 0, 0, 0};
	if( !PyArg_ParseTuple(args, "fff", &wp4.i, &wp4.j, &wp4.k) ) return 0;

	D3_FLOAT_POINT_4 sp4;
	if(!D3_Pos_WorldToScreen(&wp4, &sp4)) Py_RETURN_NONE;

	return Py_BuildValue("(fff)", sp4.i, sp4.j, sp4.k);
}

static PyMethodDef D3DMethods[] = {
	{"Sleep", D3Py_Sleep, METH_VARARGS},
	{"GetUI", D3Py_GetUI, METH_VARARGS},
	{"GetCurACD", D3Py_GetCurACD, METH_VARARGS},
	{"GetACD", D3Py_GetACD, METH_VARARGS},
	{"GetActor", D3Py_GetActor, METH_VARARGS},
	{"AH_GetItemList", D3Py_AH_GetItemList, METH_VARARGS},
	{"AH_GetInventory", D3Py_AH_GetInventory, METH_VARARGS},
	{"UseSkill", D3Py_UseSkill, METH_VARARGS},

	{"Pos_WorldToScreen", D3Py_Pos_WorldToScreen, METH_VARARGS},

	{"ReadMemory", D3Py_ReadMemory, METH_VARARGS},
	{"GetTickCount", D3Py_GetTickCount, METH_VARARGS},
	{"CallCFunction", D3Py_CallCFunction, METH_VARARGS},

    {NULL, NULL, 0}
};

static PyObject* obj_call(PyObject *self, PyObject *args, PyObject *kws)
{
	return ((D3Py_OBJECT_METHOD_PTR)self)->method(self, args, kws);
}

static PyObject* get_attr(PyObject *obj, PyObject *name)
{
	D3Py_OBJECT_PTR o = (D3Py_OBJECT_PTR)obj;
	PyObject *d = o->attrs;
	PyObject *r = PyDict_GetItem(d, name);
	if(r) {
		D3Py_LOOKUP_ATTR_DEF_PTR m = (D3Py_LOOKUP_ATTR_DEF_PTR)PyCapsule_GetPointer(r, 0);
		if(m->type == T_D3PY_OBJECT_METHOD) {
			D3Py_OBJECT_METHOD_PTR n = (D3Py_OBJECT_METHOD_PTR)alloc_pyobj(&D3Py_Object_Method_Type, -1, obj);
			n->c_this = o->c_this;
			memcpy(&n->val, &o->val, sizeof(n->val));
			n->method = m->method;
			return (PyObject*)n;

		} else if(m->type == T_D3PY_OBJECT) {
			D3Py_OBJECT_PTR n = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, m->lkid, obj);
			n->c_this = *(void**)(((char*)o->c_this) + m->offset);
			return (PyObject*)n;

		} else if(m->type == T_D3PY_OBJECT_INPLACE) {
			D3Py_OBJECT_PTR n = (D3Py_OBJECT_PTR)alloc_pyobj(&D3Py_Object_Type, m->lkid, obj);
			n->c_this = (void*)(((char*)o->c_this) + m->offset);
			return (PyObject*)n;

		} else {
			unsigned int offset_ref = m->offset_ref;
			void *ptr = o->c_this;
			if(offset_ref == -1)
				ptr = o;
			else if(offset_ref) {
				ptr = o->val[offset_ref & 0xFFFF].ptr_v;
				if(!ptr) Py_RETURN_NONE;
			}

			PyMemberDef f = {m->name, m->type, m->offset, 0, 0};
			return PyMember_GetOne((const char*)ptr, &f);

		}

	}

	return PyObject_GenericGetAttr(obj, name);
}

static PyObject * obj_repr(PyObject *o)
{
	char buf[512];
	sprintf(buf, "<PyObj: 0x%08X, CObj: 0x%08X>", o, ((D3Py_OBJECT_PTR)o)->c_this);
	return PyString_FromString(buf);
}

static PyObject* iter(PyObject *o)
{
	Py_INCREF(o);
	return o;
}

static PyObject* iter_next(PyObject *o)
{
	D3Py_ITERATOR_PTR i = (D3Py_ITERATOR_PTR)o;
	return i->next(o);
}

static void obj_dealloc(PyObject *self) 
{
	D3Py_INTERNAL *o = (D3Py_INTERNAL*)self;
	Py_XDECREF(o->dep);
	self->ob_type->tp_free(self);
}

static void init_lookup()
{
	for(int i = 0; i < LKID_END; i++) {
		PyObject *d = g_lookup[i].lookup = PyDict_New();
		D3Py_LOOKUP_ATTR_DEF_PTR ad = g_lookup[i].attr_def;
		if(!ad) continue;
		while(ad->name) {
			PyObject *o = PyCapsule_New(ad, 0, 0);
			PyDict_SetItemString(d, ad->name, o);
			Py_DECREF(o);
			ad++;
		}
	}
}


void init_D3D_module()
{
	init_lookup();

	D3Py_Iterator_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER;
	D3Py_Iterator_Type.tp_dealloc = obj_dealloc;
	D3Py_Iterator_Type.tp_iter = iter;
	D3Py_Iterator_Type.tp_iternext = iter_next;
	D3Py_Iterator_Type.tp_repr = obj_repr;
	PyType_Ready(&D3Py_Iterator_Type);

	D3Py_Object_Type.tp_flags = Py_TPFLAGS_DEFAULT;
	D3Py_Object_Type.tp_dealloc = obj_dealloc;
	D3Py_Object_Type.tp_getattro = get_attr;
	D3Py_Object_Type.tp_repr = obj_repr;
	PyType_Ready(&D3Py_Object_Type);

	D3Py_Object_Method_Type.tp_flags = Py_TPFLAGS_DEFAULT;
	D3Py_Object_Method_Type.tp_dealloc = obj_dealloc;
	D3Py_Object_Method_Type.tp_repr = obj_repr;
	D3Py_Object_Method_Type.tp_call = obj_call;
	PyType_Ready(&D3Py_Object_Method_Type);

	Py_InitModule("_D3", D3DMethods);
}
