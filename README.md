# QMultiProxyModel

The QMultiProxyModel class provides several item models as one model.
Qt4/Qt5 based.


# example:
```cpp
    QAbstractItemModel *model1 = new SomeItemModel;
    QAbstractItemModel *model2 = new SomeItemModel;
    // ...
    QAbstractItemModel *modelN = new SomeItemModel;

    QMultiProxyModel *proxy = new QMultiProxyModel;
    proxy->addSourceModel(model1);
    proxy->addSourceModel(model2);
    // ...
    proxy->addSourceModel(modelN);

    QListView view;
    view.setModel(proxy);
