#ifndef UBTEACHERGUIDECONSTANTES_H
#define UBTEACHERGUIDECONSTANTES_H

#define UBTG_SEPARATOR_FIXED_HEIGHT 3

typedef enum {
    eUBTGAddSubItemWidgetType_None,
    eUBTGAddSubItemWidgetType_Action,
    eUBTGAddSubItemWidgetType_Media,
    eUBTGAddSubItemWidgetType_Url,
    eUBTGAddSubItemWidgetType_File // Issue 1683 (Evolution) - AOU - 20131206
} eUBTGAddSubItemWidgetType;

typedef enum {
    tUBTGActionAssociateOnClickItem_NONE,
    tUBTGActionAssociateOnClickItem_URL,
    tUBTGActionAssociateOnClickItem_MEDIA,
    tUBTGActionAssociateOnClickItem_EXPAND,
    tUBTGActionAssociateOnClickItem_FILE
} tUBTGActionAssociateOnClickItem;

typedef enum {
    tUBTGTreeWidgetItemRole_HasAnAction = Qt::UserRole,
    tUBTGTreeWidgetItemRole_HasAnUrl
} tUBTGTreeWidgetItemRole;

#endif // UBTEACHERGUIDECONSTANTES_H
