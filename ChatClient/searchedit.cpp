#include "searchedit.h"

SearchEdit::SearchEdit(QWidget *parent):QLineEdit (parent),_max_len(0){
    connect(this, &QLineEdit::textChanged, this, &SearchEdit::limitTextLength);
}


void SearchEdit::SetMaxLength(int maxLen)
{
    _max_len = maxLen;
}
