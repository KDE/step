/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
   Copyright (C) 2014 Inge Wallin        <inge@lysator.liu.se>

   Step is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Step is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Step; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "toolgraphics.h"

#include "ui_configure_graph.h"
#include "ui_configure_meter.h"
#include "ui_configure_controller.h"

#include <stepcore/tool.h>
#include <stepcore/particle.h>
#include <stepcore/rigidbody.h>
#include <stepcore/solver.h>
#include <stepcore/collisionsolver.h>

#include "worldmodel.h"
#include "worldscene.h"
#include "worldfactory.h"
#include "latexformula.h"

#include <QAbstractButton>
#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFileDialog>
#include <QFocusEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QGridLayout>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QLCDNumber>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QStyleOptionGraphicsItem>
#include <QTemporaryFile>
#include <QTextDocument>
#include <QUrl>
#include <QVBoxLayout>

#include <KFontAction>
#include <KFontSizeAction>
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPlotAxis>
#include <KPlotObject>
#include <KPlotPoint>
#include <KPlotWidget>
#include <KToggleAction>
#include <KToolBar>

#include <float.h>


StepCore::Vector2d WidgetVertexHandlerGraphicsItem::value()
{
    double s = currentViewScale();
    StepCore::Vector2d size = _item->metaObject()->property(QStringLiteral("size"))->
                            readVariant(_item).value<StepCore::Vector2d>()/s;
    return (size.array()* corners[_vertexNum].array()).matrix();
}

void WidgetVertexHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    double s = currentViewScale();

    QGraphicsView* activeView = scene()->views().first();
    QTransform viewportTransform = activeView->viewportTransform();

    StepCore::Vector2d size = _item->metaObject()->property(QStringLiteral("size"))->
                        readVariant(_item).value<StepCore::Vector2d>()/s;
    StepCore::Vector2d position = _item->metaObject()->property(QStringLiteral("position"))->
                            readVariant(_item).value<StepCore::Vector2d>();

    StepCore::Vector2d oCorner = position - (size.array()*((corners[_vertexNum]).array())).matrix();

    oCorner = pointToVector( viewportTransform.inverted().map(
                QPointF(viewportTransform.map(vectorToPoint(oCorner)).toPoint()) ));

    StepCore::Vector2d delta = (value + position - oCorner)/2.0;
    StepCore::Vector2d newPos = oCorner + delta;
    newPos = pointToVector( viewportTransform.inverted().map(
                QPointF(viewportTransform.map(vectorToPoint(newPos)).toPoint()) ));
    StepCore::Vector2d newSize = (newPos - oCorner)*2.0;

    StepCore::Vector2d sign = (delta.array()*(corners[_vertexNum].array())).matrix();
    double d = -0.1/s;
    if(sign[0] < d || sign[1] < d) {
        if(sign[0] < d) {
            newPos[0] = oCorner[0]; newSize[0] = 0;
            _vertexNum ^= 1;
        }
        if(sign[1] < d) {
            newPos[1] = oCorner[1]; newSize[1] = 0;
            _vertexNum ^= 2;
        }
        _worldModel->setProperty(_item, QStringLiteral("position"), QVariant::fromValue(newPos));
        _worldModel->setProperty(_item, QStringLiteral("size"), QVariant::fromValue((newSize*s).eval()));
        setValue(value);
        return;
    }

    _worldModel->setProperty(_item, QStringLiteral("position"), QVariant::fromValue(newPos));
    _worldModel->setProperty(_item, QStringLiteral("size"), QVariant::fromValue((newSize*s).eval()));
}

WidgetGraphicsItem::WidgetGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : StepGraphicsItem(item, worldModel), _centralWidget(0)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);

    _backgroundBrush = Qt::NoBrush;

    _boundingRect = QRectF(0, 0, 0, 0);
}

WidgetGraphicsItem::~WidgetGraphicsItem()
{
    if(_centralWidget) {
        _centralWidget->hide();
        _centralWidget->deleteLater();
    }
}

OnHoverHandlerGraphicsItem* WidgetGraphicsItem::createOnHoverHandler(const QPointF& pos)
{
    double s = currentViewScale();
    StepCore::Vector2d size = _item->metaObject()->property(QStringLiteral("size"))->
                            readVariant(_item).value<StepCore::Vector2d>()/s;
    StepCore::Vector2d position = _item->metaObject()->property(QStringLiteral("position"))->
                            readVariant(_item).value<StepCore::Vector2d>();
    StepCore::Vector2d l = pointToVector(pos) - position;

    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE/s/s;
    for(unsigned int i=0; i<4; ++i) {
        double dist2 = (l - (size.array()*(WidgetVertexHandlerGraphicsItem::corners[i]).array()).matrix()).squaredNorm();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }

    if(_onHoverHandler && _onHoverHandler->vertexNum() == num)
        return _onHoverHandler;

    if(num >= 0)
        return new WidgetVertexHandlerGraphicsItem(_item, _worldModel, this, num);

    return 0;
}

// XXX: ???
void WidgetGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF&, MovingState)
{
    QGraphicsView* activeView = scene()->views().first();
    QTransform itemTransform = activeView->transform() * deviceTransform(activeView->viewportTransform());
    StepCore::Vector2d newPos = pointToVector( itemTransform.inverted().map(
                QPointF(itemTransform.map(pos/*/50.0*/).toPoint()) ))/**50.0*/;
    _worldModel->setProperty(_item, QStringLiteral("position"), QVariant::fromValue(newPos));
}

void WidgetGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    if(_isSelected) painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
    else painter->setPen(QPen(Qt::NoPen));
    painter->setBrush(_backgroundBrush);
    painter->drawRect(_boundingRect);
}

void WidgetGraphicsItem::setCenteralWidget(QWidget* widget)
{
    if(_centralWidget) {
        _centralWidget->hide();
        _centralWidget->deleteLater();
    }
    _centralWidget = widget;
    viewScaleChanged();
}

void WidgetGraphicsItem::viewScaleChanged()
{
    if(!scene() || scene()->views().isEmpty()) return;
    QGraphicsView* activeView = scene()->views().first();

    QPointF position = vectorToPoint(_item->metaObject()->property(QStringLiteral("position"))->
                    readVariant(_item).value<StepCore::Vector2d>());
    
    // Move item to the closest pixel position
    QPoint viewPosition = activeView->mapFromScene(position);
    setPos(activeView->mapToScene(viewPosition));

    StepCore::Vector2d size = _item->metaObject()->property(QStringLiteral("size"))->
                    readVariant(_item).value<StepCore::Vector2d>();

    QSize viewSize(qRound(size[0]), qRound(size[1]));
    QPoint viewTopLeft =
        viewPosition - QPoint(viewSize.width() / 2, viewSize.height() / 2);
    QRect viewRect(viewTopLeft, viewSize);
    
    QRectF sceneRect =
        activeView->mapToScene(viewRect.adjusted(0, 0, 1, 1)).boundingRect();
    QRectF boundingRect = mapRectFromScene(sceneRect);
    double s = currentViewScale();
    boundingRect.adjust(-SELECTION_MARGIN/s, -SELECTION_MARGIN/s,
                        SELECTION_MARGIN/s, SELECTION_MARGIN/s);
    
    if(boundingRect != _boundingRect) {
        prepareGeometryChange();
        _boundingRect = boundingRect;
        update();
    }
    
    // Reparent the widget if necessary.
    if(_centralWidget->parentWidget() != activeView->viewport()) {
       _centralWidget->setParent(activeView->viewport());
       _centralWidget->show();
    }

    _centralWidget->setGeometry(viewRect.adjusted(0, 0, 1, 1));
}

void WidgetGraphicsItem::stateChanged()
{
    update();
}

void WidgetGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        /*QPointF position = vectorToPoint(_item->metaObject()->property("position")->
                    readVariant(_item).value<StepCore::Vector2d>());
        setPos(position);*/
        viewScaleChanged();
        update();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

QString NoteTextEdit::emptyNotice() const
{
    return i18n("Click to enter text");
}

/*
void NoteTextEdit::focusInEvent(QFocusEvent *event)
{
    if(_noteItem->note()->text().isEmpty()) {
        ++_noteItem->_updating;
        setPlainText("");
        _noteItem->worldDataChanged(false);
        --_noteItem->_updating;
    }
    _noteItem->_hasFocus = true;
    _noteItem->_toolBar->show();
    _noteItem->setSelected(true);
    _noteItem->viewScaleChanged();
    KTextEdit::focusInEvent(event);
}

void NoteTextEdit::focusOutEvent(QFocusEvent *event)
{
    qDebug() << event->reason() << endl;
    qDebug() << QApplication::focusWidget()->metaObject()->className() << endl;

    QObject* f = QApplication::focusWidget();
    if(f == this) f = NULL;
    while(f && f != _noteItem->_widget) f = f->parent();

    if(!f && event->reason() != Qt::PopupFocusReason) {
        if(_noteItem->note()->text().isEmpty()) {
            ++_noteItem->_updating;
            setPlainText(emptyNotice());
            _noteItem->worldDataChanged(false);
            --_noteItem->_updating;
        }
        _noteItem->_hasFocus = false;
        _noteItem->_toolBar->hide();
        _noteItem->viewScaleChanged();
    }
    KTextEdit::focusOutEvent(event);
}
*/

StepCore::NoteFormula* NoteTextEdit::formulaAt(const QPoint& pos)
{
    int p = document()->documentLayout()->hitTest(pos, Qt::ExactHit);
    if(p < 0) return NULL;

    QTextCursor cursor(document());
    cursor.setPosition(p);
    if(cursor.atEnd()) return NULL;
    cursor.setPosition(p+1);

    QTextFormat format = cursor.charFormat();
    if(!format.isImageFormat()) return NULL;
    QString image = format.toImageFormat().name();

    StepCore::Item* item = _noteItem->note()->childItem(image);
    if(!item) {
        foreach(StepCore::Item* it, _noteItem->_newItems)
            if(it->name() == image) { item = it; break; }
    }

    if(!item || !item->metaObject()->inherits<StepCore::NoteFormula>())
        return NULL;

    return static_cast<StepCore::NoteFormula*>(item);
}

void NoteTextEdit::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton) _mousePressPoint = e->pos();
    KTextEdit::mousePressEvent(e);
}

void NoteTextEdit::mouseMoveEvent(QMouseEvent *e)
{
    if(formulaAt(e->pos()) != NULL) {
        viewport()->setCursor(Qt::PointingHandCursor);
    } else {
        viewport()->setCursor(Qt::IBeamCursor);
    }
    _mousePressPoint.setX(-1);
    KTextEdit::mouseMoveEvent(e);
}

void NoteTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton && e->pos() == _mousePressPoint) {
        StepCore::NoteFormula* formula = formulaAt(e->pos());
        if(formula) {
            e->accept();
            _noteItem->editFormula(formula);
            setDocument(document());
            return;
        }
    }
    KTextEdit::mouseReleaseEvent(e);
}

NoteGraphicsItem::NoteGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WidgetGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Note*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);

    _widget = new QWidget();
    _widget->setPalette(QPalette(Qt::lightGray));

    _textEdit = new NoteTextEdit(this, _widget);
    _textEdit->setFrameShape(QFrame::NoFrame);
    QPalette p = _textEdit->palette();
    p.setColor(QPalette::Base, Qt::transparent);
    _textEdit->setPalette(p);
    //_textEdit->setStyleSheet(".NoteTextEdit {background-color: rgba(0,0,0,0%);}");

    _toolBar = new KToolBar(_widget);
    _toolBar->setIconDimensions(16);
    _toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _toolBar->setContentsMargins(0,0,0,0);
    if(_toolBar->layout()) _toolBar->layout()->setSpacing(0);
    //_toolBar->setStyleSheet(".KToolBar {margin: 0px; border-width: 0px; padding: 0px; }");

    _actionColor = new QAction(QIcon(), i18n("&Color"), _toolBar);

    _actionBold = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-bold")), i18n("&Bold"), _toolBar);
    _actionBold->setShortcut(Qt::CTRL | Qt::Key_B);
    _actionItalic = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-italic")), i18n("&Italic"), _toolBar);
    _actionItalic->setShortcut(Qt::CTRL | Qt::Key_I);
    _actionUnderline = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-underline")), i18n("&Underline"), _toolBar);
    _actionUnderline->setShortcut(Qt::CTRL | Qt::Key_U);

    _actionAlignLeft = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-left")), i18n("Align &Left"), _toolBar);
    _actionAlignLeft->setShortcut(Qt::CTRL | Qt::Key_L);
    _actionAlignCenter = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-center")), i18n("Align C&enter"), _toolBar);
    _actionAlignCenter->setShortcut(Qt::CTRL | Qt::Key_E);
    _actionAlignRight = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-right")), i18n("Align &Right"), _toolBar);
    _actionAlignRight->setShortcut(Qt::CTRL | Qt::Key_R);
    _actionAlignJustify = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-fill")), i18n("Align &Justify"), _toolBar);
    _actionAlignJustify->setShortcut(Qt::CTRL | Qt::Key_J);

    _actionAlign = new KSelectAction(i18n("&Align"), _toolBar);
    _actionAlign->setToolBarMode(KSelectAction::MenuMode);
    _actionAlign->setToolButtonPopupMode(QToolButton::InstantPopup);
    _actionAlign->addAction(_actionAlignLeft);
    _actionAlign->addAction(_actionAlignCenter);
    _actionAlign->addAction(_actionAlignRight);
    _actionAlign->addAction(_actionAlignJustify);
    _actionAlignLeft->setChecked(true);

    _actionFont = new KFontAction(i18n("&Font"), _toolBar);
    _actionFontSize = new KFontSizeAction(i18n("Font &Size"), _toolBar);

    _actionInsertImage = new QAction(QIcon::fromTheme(QStringLiteral("insert-image")), i18n("Insert &Image"), _toolBar);
#ifdef __GNUC__
#warning Select right icon here
#endif
    _actionInsertFormula = new QAction(QIcon::fromTheme(QStringLiteral("application-vnd.oasis.opendocument.formula")),
                                    i18n("Insert &Formula"), _toolBar);

    connect(_actionColor, &QAction::triggered, this, &NoteGraphicsItem::formatColor);
    connect(_actionBold, &QAction::triggered, this, &NoteGraphicsItem::formatBold);
    connect(_actionItalic, &QAction::triggered, _textEdit, &QTextEdit::setFontItalic);
    connect(_actionUnderline, &QAction::triggered, _textEdit, &QTextEdit::setFontUnderline);
    connect(_actionAlign, SIGNAL(triggered(QAction*)), this, SLOT(formatAlign(QAction*)));
    connect(_actionFont, SIGNAL(triggered(QString)), this, SLOT(formatFontFamily(QString)));
    connect(_actionFontSize, &KFontSizeAction::fontSizeChanged, this, &NoteGraphicsItem::formatFontSize);
    connect(_actionInsertImage, &QAction::triggered, this, &NoteGraphicsItem::insertImage);
    connect(_actionInsertFormula, &QAction::triggered, this, &NoteGraphicsItem::insertFormula);
    
    connect(_textEdit, &QTextEdit::currentCharFormatChanged,
                            this, &NoteGraphicsItem::currentCharFormatChanged);
    connect(_textEdit, &QTextEdit::cursorPositionChanged, this, &NoteGraphicsItem::cursorPositionChanged);


    connect(_toolBar, SIGNAL(actionTriggered(QAction*)), _textEdit, SLOT(setFocus()));

    _toolBar->addAction(_actionAlign);
    _toolBar->addAction(_actionBold);
    _toolBar->addAction(_actionItalic);
    _toolBar->addAction(_actionUnderline);
    _toolBar->addAction(_actionColor);
    //_toolBar->addSeparator();

    //_toolBar->addSeparator();

    _toolBar->addAction(_actionInsertImage);
    _toolBar->addAction(_actionInsertFormula);

    _toolBar->addAction(_actionFontSize);
    _toolBar->addAction(_actionFont);

    QVBoxLayout* layout = new QVBoxLayout(_widget);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(_textEdit);
    layout->addWidget(_toolBar);

    // without it focus is passed to QGrahicsView
    _toolBar->setFocusPolicy(Qt::ClickFocus);
    _toolBar->setFocusProxy(_textEdit);
    _widget->setFocusProxy(_textEdit);

    _hasFocus = false;
    _toolBar->hide();

    _textEdit->installEventFilter(this);

    QComboBox* font = qobject_cast<QComboBox*>(_toolBar->widgetForAction(_actionFont));
    if(font) {
        font->setMinimumContentsLength(5);
        font->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        font->installEventFilter(this);
        font->setToolTip(_actionFont->toolTip());
    }

    QComboBox* fontSize = qobject_cast<QComboBox*>(_toolBar->widgetForAction(_actionFontSize));
    if(fontSize) {
        fontSize->setMinimumContentsLength(2);
        fontSize->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        fontSize->installEventFilter(this);
        fontSize->setToolTip(_actionFontSize->toolTip());
    }

    setCenteralWidget(_widget);
    setOnHoverHandlerEnabled(true);
    _widget->setMouseTracking(true);
    _textEdit->setMouseTracking(true);
    _toolBar->setMouseTracking(true);
}

inline StepCore::Note* NoteGraphicsItem::note() const
{
    return static_cast<StepCore::Note*>(_item);
}

bool NoteGraphicsItem::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::FocusIn) {
        if(!_hasFocus) {
            _hasFocus = true;
            if(note()->text().isEmpty()) {
                //++_updating;
                _textEdit->setPlainText(QLatin1String(""));
                worldDataChanged(false);
                //--_updating;
            }
            _toolBar->show();
            bool multiSelect = (QApplication::keyboardModifiers() & Qt::ControlModifier) != 0;
            if(!multiSelect/* && !isSelected()*/) {
                if(scene()) scene()->clearSelection();
                _worldModel->selectionModel()->clearSelection();
            }
            setSelected(true);
            viewScaleChanged();
        }
    } else if(event->type() == QEvent::FocusOut &&
            static_cast<QFocusEvent*>(event)->reason() != Qt::PopupFocusReason) {

        QObject* f = QApplication::focusWidget();
        if(f == obj) f = NULL;
        while(f && f != _widget) f = f->parent();

        if(!f) {
            _worldModel->simulationPause();

            QString newText = _textEdit->toHtml();
            if(newText != note()->text()) {
                //++_updating;
                _worldModel->beginMacro(i18n("Edit %1", _item->name()));

                foreach(StepCore::Item* item, note()->items())
                    if(!newText.contains(item->name())) _worldModel->deleteItem(item);

                foreach(StepCore::Item* item, _newItems)
                    if(!newText.contains(item->name())) _newItems.removeAll(item);

                foreach(StepCore::Item* item, _newItems)
                    _worldModel->addItem(item, note());

                _newItems.clear();

                _worldModel->setProperty(_item, QStringLiteral("text"), newText);

                _worldModel->endMacro();
                //--_updating;
            }

            _hasFocus = false;
            _toolBar->hide();
            _textEdit->clear();

            viewScaleChanged();
            worldDataChanged(false);
        }
    }
    return QObject::eventFilter(obj, event);
}

void NoteGraphicsItem::formatColor()
{
    QColor color = _textEdit->textColor();
    color = QColorDialog::getColor(color, _widget);
    if(QColorDialog::Accepted && color.isValid()) {
        _textEdit->setTextColor(color);
    }
}

void NoteGraphicsItem::formatBold(bool checked)
{
    _textEdit->setFontWeight(checked ? QFont::Bold : QFont::Normal);
}

void NoteGraphicsItem::formatAlign(QAction* action)
{
    if(action == _actionAlignLeft)
        _textEdit->setAlignment(Qt::AlignLeft);
    else if(action == _actionAlignCenter)
        _textEdit->setAlignment(Qt::AlignHCenter);
    else if(action == _actionAlignRight)
        _textEdit->setAlignment(Qt::AlignRight);
    else if(action == _actionAlignJustify)
        _textEdit->setAlignment(Qt::AlignJustify);

    _actionAlign->setIcon(action->icon());
}

void NoteGraphicsItem::formatFontFamily(const QString& family)
{
    _textEdit->setFontFamily(family);
    _textEdit->setFocus();
}

void NoteGraphicsItem::formatFontSize(int size)
{
    if(size > 0) _textEdit->setFontPointSize(size);
    else currentCharFormatChanged(_textEdit->currentCharFormat());
    _textEdit->setFocus();
}

void NoteGraphicsItem::currentCharFormatChanged(const QTextCharFormat& f)
{
    _actionBold->setChecked(f.fontWeight() >= QFont::Bold);
    _actionItalic->setChecked(f.fontItalic());
    _actionUnderline->setChecked(f.fontUnderline());

    QPixmap pix(16,16);
    pix.fill(_textEdit->textColor());
    _actionColor->setIcon(pix);

    QFontInfo ff(f.font());
#ifdef __GNUC__
#warning Strange, the following line does nothing !
#endif
    _actionFont->setFont(ff.family());
    _actionFontSize->setFontSize(ff.pointSize());
}

void NoteGraphicsItem::cursorPositionChanged()
{
    if(_textEdit->alignment() & Qt::AlignLeft)
        _actionAlignLeft->setChecked(true);
    else if(_textEdit->alignment() & Qt::AlignHCenter)
        _actionAlignCenter->setChecked(true);
    else if(_textEdit->alignment() & Qt::AlignRight)
        _actionAlignRight->setChecked(true);
    else if(_textEdit->alignment() & Qt::AlignJustify)
        _actionAlignJustify->setChecked(true);

    _actionAlign->setIcon(_actionAlign->currentAction()->icon());
}

void NoteGraphicsItem::insertImage()
{
    QUrl url = QFileDialog::getOpenFileUrl(_widget, i18n("Open Image File"), QUrl(), i18n("Images (*.png *.jpg *.jpeg)"));
    if(url.isEmpty()) return;

    QTemporaryFile tempFile;
    tempFile.open();
    KIO::FileCopyJob *job = KIO::file_copy(url, QUrl::fromLocalFile(tempFile.fileName()), -1, KIO::Overwrite);
    KJobWidgets::setWindow(job, _widget);
    job->exec();
    if (job->error()) {
        KMessageBox::error(_widget, job->errorString());
        return;
    }

    QByteArray data = tempFile.readAll();
    tempFile.close();

    QPixmap pixmap;
    if(!pixmap.loadFromData(data)) {
        KMessageBox::error(_widget, i18n("Cannot parse file '%1'", tempFile.fileName()));
        return;
    }

    QString imgName;
    for(int n=0;; ++n) {
        imgName = QStringLiteral("img:%1").arg(n);
        if(note()->childItem(imgName) != NULL) continue;
        bool found = false;
        foreach(StepCore::Item* item, _newItems)
            if(item->name() == imgName) { found = true; break; }
        if(!found) break;
    }
    
    _newItems << new StepCore::NoteImage(imgName, data);
    //_textEdit->document()->addResource(QTextDocument::ImageResource, imgName, pixmap);
    _textEdit->insertHtml(QStringLiteral("<img src=\"%1\" />").arg(imgName));
}

void NoteGraphicsItem::insertFormula()
{
    QString imgName;
    for(int n=0;; ++n) {
        imgName = QStringLiteral("fml:%1").arg(n);
        if(note()->childItem(imgName) != NULL) continue;
        bool found = false;
        foreach(StepCore::Item* item, _newItems)
            if(item->name() == imgName) { found = true; break; }
        if(!found) break;
    }

    StepCore::NoteFormula* formula = new StepCore::NoteFormula(imgName);
    if(!editFormula(formula)) {
        delete formula;
        return;
    }

    _newItems << formula;
    _textEdit->insertHtml(QStringLiteral("<img src=\"%1\" />").arg(imgName));
}

bool NoteGraphicsItem::editFormula(StepCore::NoteFormula* formula)
{
    if(!LatexFormula::isLatexInstalled()) {
        KMessageBox::sorry(_widget, i18n("Cannot find latex installation. "
                    "You need 'latex', 'dvips' and 'gs' executables installed and accessible from $PATH"));
        return false;
    }

    bool ok;
    QString code = QInputDialog::getMultiLineText(_widget, i18n("LaTex Formula - Step"),
                i18n("Enter LaTeX formula string"), QString(formula->code()), &ok);
    if(!ok) return false;

    QByteArray image;
    QString error;

    bool result = LatexFormula::compileFormula(code, &image, &error);

    if(!result) {
        KMessageBox::error(_widget, i18n("Cannot compile LaTeX formula: %1", error));
        return false;
    }

    QPixmap pixmap;
    if(!pixmap.loadFromData(image)) {
        KMessageBox::error(_widget, i18n("Cannot parse result image"));
        return false;
    }

    formula->setCode(code);
    formula->setImage(image);

    _textEdit->document()->addResource(QTextDocument::ImageResource,
                                            QUrl(formula->name()), pixmap);
    return true;
}

void NoteGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        if(!_hasFocus && _textEdit->toHtml() != note()->text()) {
            //++_updating;

            const StepCore::ItemList::const_iterator end = note()->items().end();
            for(StepCore::ItemList::const_iterator it = note()->items().begin(); it != end; ++it) {
                if((*it)->metaObject()->inherits<StepCore::NoteImage>()) {
                    QPixmap pix;
                    pix.loadFromData(static_cast<StepCore::NoteImage*>(*it)->image());
                    _textEdit->document()->addResource(QTextDocument::ImageResource, QUrl((*it)->name()), pix);
                }
            }

            /*
            const StepCore::NoteDataMap::const_iterator end = note()->dataMap().constEnd();
            for(StepCore::NoteDataMap::const_iterator it = note()->dataMap().constBegin();
                                                                            it != end; ++it) {
                QPixmap pix;
                pix.loadFromData(it.value());
                _textEdit->document()->addResource(QTextDocument::ImageResource, it.key(), pix);
            }
            */

            if(!_textEdit->hasFocus() && note()->text().isEmpty()) {
                _textEdit->setPlainText(_textEdit->emptyNotice());
            } else {
                _textEdit->setHtml(note()->text());
            }

            currentCharFormatChanged(_textEdit->currentCharFormat());
            cursorPositionChanged();
            //--_updating;
        }
        WidgetGraphicsItem::worldDataChanged(dynamicOnly);
    }
}

////////////////////////////////////////////////////
DataSourceWidget::DataSourceWidget(QWidget* parent)
    : QWidget(parent), _worldModel(0)
{
    _skipReadOnly = false;
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    _object = new KComboBox(this);
    _object->setToolTip(i18n("Object name"));
    _object->setMinimumContentsLength(10);
    layout->addWidget(_object, 1);

    _property = new KComboBox(this);
    _property->setToolTip(i18n("Property name"));
    _property->setEnabled(false);
    _property->setMinimumContentsLength(10);
    layout->addWidget(_property, 1);

    _index = new KComboBox(this);
    _index->setToolTip(i18n("Vector index"));
    _index->setMinimumContentsLength(1);
    _index->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    layout->addWidget(_index, 0);

    connect(_object, SIGNAL(activated(int)),
            this, SLOT(objectSelected(int)));
    connect(_property, SIGNAL(activated(int)),
            this, SLOT(propertySelected(int)));

    connect(_object, SIGNAL(activated(int)),
            this, SIGNAL(dataSourceChanged()));
    connect(_property, SIGNAL(activated(int)),
            this, SIGNAL(dataSourceChanged()));
    connect(_index, SIGNAL(activated(int)),
            this, SIGNAL(dataSourceChanged()));
}

void DataSourceWidget::addObjects(const QModelIndex& parent, const QString& indent)
{
    for(int i=0; i<_worldModel->rowCount(parent); ++i) {
        QModelIndex index = _worldModel->index(i, 0, parent);
        QString name = index.data(WorldModel::FormattedNameRole).toString();
        _object->addItem(indent + name, QVariant::fromValue(_worldModel->object(index)));
        addObjects(index, indent + ' ');
    }
}

StepCore::Object* DataSourceWidget::dataObject() const
{
    if(_object->currentIndex() < 0) return NULL;
    return _object->itemData(_object->currentIndex()).value<StepCore::Object*>();
}

void DataSourceWidget::setDataSource(WorldModel* worldModel,
            StepCore::Object* object, const QString& property, int index)
{
    _worldModel = worldModel;
    if(!_worldModel) return;

    _object->clear();

    addObjects(QModelIndex(), QLatin1String(""));

    int objIndex = _object->findData(QVariant::fromValue(object));
    _object->setCurrentIndex( objIndex );
    objectSelected(objIndex);

    int propIndex = _property->findData(property);
    _property->setCurrentIndex( propIndex );
    propertySelected(propIndex);

    _index->setCurrentIndex( index );
}

void DataSourceWidget::objectSelected(int index)
{
    Q_ASSERT(_worldModel);

    _property->clear();

    const StepCore::Object* obj = _object->itemData(index).value<StepCore::Object*>();
    if(obj != 0) {
        _property->setEnabled(true);
        for(int i=0; i<obj->metaObject()->propertyCount(); ++i) {
            const StepCore::MetaProperty* pr = obj->metaObject()->property(i);
            if(_skipReadOnly && !pr->isWritable()) continue;
            if(pr->userTypeId() == qMetaTypeId<double>() ||
                        pr->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
                _property->addItem(pr->nameTr(), pr->name());
            }
        }
        propertySelected(_property->currentIndex());
    } else {
        _property->setEnabled(false);
    }
}

void DataSourceWidget::propertySelected(int index)
{
    Q_ASSERT(_worldModel);

    QString text = _property->itemData(index).toString();
    const StepCore::Object* obj = _object->itemData(_object->currentIndex())
                                                        .value<StepCore::Object*>();
    const StepCore::MetaProperty* pr = obj ? obj->metaObject()->property(text) : 0;

    _index->clear();
    if(pr != 0 && pr->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
        _index->setEnabled(true);
        _index->addItem(QStringLiteral("0"));
        _index->addItem(QStringLiteral("1"));
    } else {
        _index->setEnabled(false);
    }
}

////////////////////////////////////////////////////
GraphGraphicsItem::GraphGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WidgetGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Graph*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);

    _plotWidget = new KPlotWidget();
    _plotWidget->setPalette(QPalette(Qt::lightGray));
    _plotWidget->setBackgroundColor(Qt::white);
    _plotWidget->setForegroundColor(Qt::black);
    //_plotWidget->setLeftPadding(0);
    //_plotWidget->setTopPadding(2);
    //_plotWidget->setRightPadding(3);

    _plotObject = new KPlotObject(Qt::black);
    _plotObject->setShowPoints(false);
    _plotObject->setShowLines(true);
    _plotObject->setPointStyle(KPlotObject::Square);

    _plotObject1 = new KPlotObject(Qt::red);
    _plotObject1->setShowPoints(true);
    _plotObject1->setShowLines(false);
    _plotObject1->setPointStyle(KPlotObject::Square);

    QList<KPlotObject*> plotObjects;
    plotObjects << _plotObject;
    plotObjects << _plotObject1;

    //_plotWidget->setAntialiasing(true);
    _plotWidget->addPlotObjects(plotObjects);

    _lastColor = 0xff000000;
    _lastPointTime = -HUGE_VAL;

    setCenteralWidget(_plotWidget);

    setOnHoverHandlerEnabled(true);
    _plotWidget->setMouseTracking(true);
}

inline StepCore::Graph* GraphGraphicsItem::graph() const
{
    return static_cast<StepCore::Graph*>(_item);
}

void GraphGraphicsItem::adjustLimits()
{
    double minX =  HUGE_VAL, minY =  HUGE_VAL;
    double maxX = -HUGE_VAL, maxY = -HUGE_VAL;

    if(graph()->autoLimitsX() || graph()->autoLimitsY()) {
        for(int i=0; i<(int) graph()->points().size(); ++i) {
            StepCore::Vector2d p = graph()->points()[i];
            if(p[0] < minX) minX = p[0];
            if(p[0] > maxX) maxX = p[0];
            if(p[1] < minY) minY = p[1];
            if(p[1] > maxY) maxY = p[1];
        }
    }

    if(!graph()->autoLimitsX() || graph()->points().empty()) {
        minX = graph()->limitsX()[0];
        maxX = graph()->limitsX()[1];
    } else {
        double range = maxX - minX;
        if(range != 0) { minX -= 0.1*range; maxX += 0.1*range; }
        else { minX -= 0.5; maxX += 0.5; }
    }

    if(!graph()->autoLimitsY() || graph()->points().empty()) {
        minY = graph()->limitsY()[0];
        maxY = graph()->limitsY()[1];
    } else {
        double range = maxY - minY;
        if(range != 0) { minY -= 0.1*range; maxY += 0.1*range; }
        else { minY -= 0.5; maxY += 0.5; }
    }

    _plotWidget->setLimits(minX, maxX, minY, maxY);
}

void GraphGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        viewScaleChanged();

        // Labels
        QString labelX, labelY;
        if(graph()->isValidX()) {
            labelX = i18n("%1.%2", _worldModel->formatName(graph()->objectX()), graph()->propertyX());
            if(graph()->indexX() >= 0) labelX.append(i18n("[%1]", graph()->indexX()));
            QString units = graph()->unitsX();
            if(!units.isEmpty()) labelX.append(" [").append(units).append("]");
        } else {
            labelX = i18n("[not configured]");
        }
        if(graph()->isValidY()) {
            labelY = i18n("%1.%2", _worldModel->formatName(graph()->objectY()), graph()->propertyY());
            if(graph()->indexY() >= 0) labelY.append(i18n("[%1]", graph()->indexY()));
            QString units = graph()->unitsY();
            if(!units.isEmpty()) labelY.append(" [").append(units).append("]");
        } else {
            labelY = i18n("[not configured]");
        }
        _plotWidget->axis( KPlotWidget::BottomAxis )->setLabel(labelX);
        _plotWidget->axis( KPlotWidget::LeftAxis )->setLabel(labelY);

        if(!graph()->autoLimitsX() && !graph()->autoLimitsY()) adjustLimits();

        _plotObject->setShowPoints(graph()->showPoints());
        _plotObject->setShowLines(graph()->showLines());

        if(_lastColor != graph()->color()) {
            _lastColor = graph()->color();
            _plotObject->setLinePen(QPen(QColor::fromRgba(_lastColor),0));
        }

        /*
        // Points
        _plotObject->clearPoints();
        for(int i=0; i<(int) graph()->points().size(); ++i) {
            StepCore::Vector2d p = graph()->points()[i];
            _plotObject->addPoint(p[0], p[1]);
        }

        adjustLimits();
        _plotWidget->update();
        */

        WidgetGraphicsItem::worldDataChanged(dynamicOnly);
    }
    
    if(_worldModel->isSimulationActive()) {
        if(_worldModel->world()->time() > _lastPointTime
                    + 1.0/_worldModel->simulationFps() - 1e-2/_worldModel->simulationFps()) {
            graph()->recordPoint();
            _lastPointTime = _worldModel->world()->time();
        }
    }

    int po_count, p_count;
    do {
        const QList<KPlotPoint*> points = _plotObject->points();
        po_count = points.count(); p_count = graph()->points().size();
        int count = qMin(po_count, p_count);
        for(int p=0; p < count; ++p)
            points[p]->setPosition(vectorToPoint(graph()->points()[p]));
    } while(0);

    if(po_count < p_count) {
        for(; po_count < p_count; ++po_count)
            _plotObject->addPoint(vectorToPoint(graph()->points()[po_count]));
    } else {
        for(--po_count; po_count >= p_count; --po_count)
            _plotObject->removePoint(po_count);
    }

    if(p_count > 0) {
        if(_plotObject1->points().isEmpty()) {
            _plotObject1->addPoint(0,0);
        }
        _plotObject1->points()[0]->setPosition(vectorToPoint(graph()->points()[p_count-1]));
    } else {
        _plotObject1->clearPoints();
    }

    if(graph()->autoLimitsX() || graph()->autoLimitsY()) adjustLimits();
    _plotWidget->update();

#if 0
//#error Do setProperty here and remove DynamicOnly from points
        if(ok) {
            _plotObject->addPoint(point[0], point[1]);
            if(graph()->autoLimitsX() || graph()->autoLimitsY()) 
                adjustLimits();
            _plotWidget->update();
        }
        _lastPointTime = _worldModel->world()->time();
        worldDataChanged(false);
        //_worldModel->setProperty(graph(), "name", QString("test"));
    }
#endif
}

void GraphMenuHandler::populateMenu(QMenu* menu, KActionCollection* actions)
{
    _confUi = 0;
    _confDialog = 0;
    _confChanged = false;

    menu->addAction(QIcon::fromTheme(QStringLiteral("edit-clear")), i18n("Clear graph"), this, &GraphMenuHandler::clearGraph);
    menu->addAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure graph..."), this, &GraphMenuHandler::configureGraph);
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu, actions);
}

inline StepCore::Graph* GraphMenuHandler::graph() const
{
    return static_cast<StepCore::Graph*>(_object);
}

void GraphMenuHandler::configureGraph()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _confChanged = false;
    _confDialog = new QDialog(); // XXX: parent?
    
    _confDialog->setWindowTitle(i18n("Configure graph"));
    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
				      | QDialogButtonBox::Cancel
				      | QDialogButtonBox::Apply);
    QWidget *mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    _confDialog->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    QPushButton *okButton = _buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    _confDialog->connect(_buttonBox, &QDialogButtonBox::accepted, _confDialog, &QDialog::accept);
    _confDialog->connect(_buttonBox, &QDialogButtonBox::rejected, _confDialog, &QDialog::reject);
    mainLayout->addWidget(_buttonBox);

    _confUi = new Ui::WidgetConfigureGraph;
    _confUi->setupUi(mainWidget);

    _confUi->dataSourceX->setDataSource(_worldModel,
                    graph()->objectX(), graph()->propertyX(), graph()->indexX());
    _confUi->dataSourceY->setDataSource(_worldModel,
                    graph()->objectY(), graph()->propertyY(), graph()->indexY());

    _confUi->checkBoxAutoX->setChecked(graph()->autoLimitsX());
    _confUi->checkBoxAutoY->setChecked(graph()->autoLimitsY());

    _confUi->lineEditMinX->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMinX));
    _confUi->lineEditMaxX->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMaxX));
    _confUi->lineEditMinY->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMinY));
    _confUi->lineEditMaxY->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMaxY));

    _confUi->lineEditMinX->setText(QString::number(graph()->limitsX()[0]));
    _confUi->lineEditMaxX->setText(QString::number(graph()->limitsX()[1]));
    _confUi->lineEditMinY->setText(QString::number(graph()->limitsY()[0]));
    _confUi->lineEditMaxY->setText(QString::number(graph()->limitsY()[1]));

    _confUi->checkBoxShowLines->setChecked(graph()->showLines());
    _confUi->checkBoxShowPoints->setChecked(graph()->showPoints());

    _buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    connect(_buttonBox, &QDialogButtonBox::clicked, this, &GraphMenuHandler::confApply);

    connect(_confUi->dataSourceX, &DataSourceWidget::dataSourceChanged, this, &GraphMenuHandler::confChanged);
    connect(_confUi->dataSourceY, &DataSourceWidget::dataSourceChanged, this, &GraphMenuHandler::confChanged);
    connect(_confUi->checkBoxAutoX, &QCheckBox::stateChanged, this, &GraphMenuHandler::confChanged);
    connect(_confUi->checkBoxAutoY, &QCheckBox::stateChanged, this, &GraphMenuHandler::confChanged);
    connect(_confUi->lineEditMinX, &QLineEdit::textEdited, this, &GraphMenuHandler::confChanged);
    connect(_confUi->lineEditMaxX, &QLineEdit::textEdited, this, &GraphMenuHandler::confChanged);
    connect(_confUi->lineEditMinY, &QLineEdit::textEdited, this, &GraphMenuHandler::confChanged);
    connect(_confUi->lineEditMaxY, &QLineEdit::textEdited, this, &GraphMenuHandler::confChanged);
    connect(_confUi->checkBoxShowLines, &QCheckBox::stateChanged, this, &GraphMenuHandler::confChanged);
    connect(_confUi->checkBoxShowPoints, &QCheckBox::stateChanged, this, &GraphMenuHandler::confChanged);

    _confDialog->exec();

    delete _confDialog; _confDialog = 0;
    delete _confUi; _confUi = 0;
}

void GraphMenuHandler::confApply(QAbstractButton *button)
{
    if (_buttonBox->button(QDialogButtonBox::Apply) != button
	&& _buttonBox->button(QDialogButtonBox::Ok) != button) {
	return;
    }

    Q_ASSERT(_confUi && _confDialog);

    // XXX: check for actual change ?
    if(!_confChanged) return;
    _worldModel->beginMacro(i18n("Edit properties of %1", graph()->name()));

    QVariant objX = QVariant::fromValue(_confUi->dataSourceX->dataObject());

    QVariant objY = QVariant::fromValue(_confUi->dataSourceY->dataObject());

    _worldModel->setProperty(graph(), QStringLiteral("objectX"), objX);
    _worldModel->setProperty(graph(), QStringLiteral("propertyX"),
                        _confUi->dataSourceX->dataProperty());
    _worldModel->setProperty(graph(), QStringLiteral("indexX"),
                        _confUi->dataSourceX->dataIndex());

    _worldModel->setProperty(graph(), QStringLiteral("objectY"), objY);
    _worldModel->setProperty(graph(), QStringLiteral("propertyY"),
                        _confUi->dataSourceY->dataProperty());
    _worldModel->setProperty(graph(), QStringLiteral("indexY"),
                        _confUi->dataSourceY->dataIndex());

    _worldModel->setProperty(graph(), QStringLiteral("autoLimitsX"),
                        _confUi->checkBoxAutoX->isChecked());
    _worldModel->setProperty(graph(), QStringLiteral("autoLimitsY"),
                        _confUi->checkBoxAutoY->isChecked());

    StepCore::Vector2d limitsX(_confUi->lineEditMinX->text().toDouble(),
                               _confUi->lineEditMaxX->text().toDouble());
    StepCore::Vector2d limitsY(_confUi->lineEditMinY->text().toDouble(),
                               _confUi->lineEditMaxY->text().toDouble());

    _worldModel->setProperty(graph(), QStringLiteral("limitsX"),
                        QVariant::fromValue(limitsX));
    _worldModel->setProperty(graph(), QStringLiteral("limitsY"),
                        QVariant::fromValue(limitsY));

    _worldModel->setProperty(graph(), QStringLiteral("showLines"),
                        _confUi->checkBoxShowLines->isChecked());
    _worldModel->setProperty(graph(), QStringLiteral("showPoints"),
                        _confUi->checkBoxShowPoints->isChecked());

    _worldModel->endMacro();
}

void GraphMenuHandler::confChanged()
{
    Q_ASSERT(_confUi && _confDialog);
    _confChanged = true;
    _buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void GraphMenuHandler::clearGraph()
{
    _worldModel->simulationPause();
    //_lastPointTime = -HUGE_VAL; // XXX
    _worldModel->beginMacro(i18n("Clear graph %1", _object->name()));
    _worldModel->setProperty(graph(), QStringLiteral("points"),
                   QVariant::fromValue(StepCore::Vector2dList()) );
    _worldModel->endMacro();
}

////////////////////////////////////////////////////
MeterGraphicsItem::MeterGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WidgetGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Meter*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);
    setBackgroundBrush(QBrush(Qt::white));

    _widget = new QFrame();
    _widget->setFrameShape(QFrame::Box);
    _widget->setPalette(QPalette(Qt::lightGray));

    QGridLayout* layout = new QGridLayout(_widget);
    layout->setContentsMargins(0,0,2,0);
    layout->setSpacing(0);

    _lcdNumber = new QLCDNumber(_widget);
    _lcdNumber->setFrameShape(QFrame::NoFrame);
    _lcdNumber->setSegmentStyle(QLCDNumber::Flat);
    _lcdNumber->display(0);

    _labelUnits = new QLabel(_widget);
    _labelUnits->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(_lcdNumber, 0, 0, 1, 1);
    layout->addWidget(_labelUnits, 0, 1, 1, 1);

    setCenteralWidget(_widget);
    setOnHoverHandlerEnabled(true);
    _widget->setMouseTracking(true);
    _lcdNumber->setMouseTracking(true);
    _labelUnits->setMouseTracking(true);
}

inline StepCore::Meter* MeterGraphicsItem::meter() const
{
    return static_cast<StepCore::Meter*>(_item);
}

void MeterGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        viewScaleChanged();

        if(meter()->digits() != _lcdNumber->digitCount())
            _lcdNumber->setDigitCount(meter()->digits());

        QString units = meter()->units();
        if(units != _labelUnits->text()) {
            QFont font(_labelUnits->font());
            int pixelSize = int(meter()->size()[1]/2);
            for(; pixelSize > 0; --pixelSize) {
                font.setPixelSize(pixelSize);
                QFontMetrics fm(font);
                if(fm.boundingRect(units).width() < int(meter()->size()[0]/3)) break;
            }
            _labelUnits->setFont(font);
            _labelUnits->setText(units);
        }

        WidgetGraphicsItem::worldDataChanged(dynamicOnly);
    }

    double value = meter()->value();
    _lcdNumber->display(value);
}

void MeterMenuHandler::populateMenu(QMenu* menu, KActionCollection* actions)
{
    _confUi = 0;
    _confDialog = 0;
    _confChanged = false;

    menu->addAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure meter..."), this, &MeterMenuHandler::configureMeter);
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu, actions);
}

inline StepCore::Meter* MeterMenuHandler::meter() const
{
    return static_cast<StepCore::Meter*>(_object);
}

void MeterMenuHandler::configureMeter()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _confChanged = false;

    _confDialog = new QDialog(); // XXX
    _confDialog->setWindowTitle(i18n("Configure meter"));
    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
				      | QDialogButtonBox::Cancel
				      | QDialogButtonBox::Apply);
    QWidget *mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    _confDialog->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    QPushButton *okButton = _buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    _confDialog->connect(_buttonBox, &QDialogButtonBox::accepted, _confDialog, &QDialog::accept);
    _confDialog->connect(_buttonBox, &QDialogButtonBox::rejected, _confDialog, &QDialog::reject);
    mainLayout->addWidget(_buttonBox);

    _confUi = new Ui::WidgetConfigureMeter;
    _confUi->setupUi(mainWidget);

    _confUi->dataSource->setDataSource(_worldModel,
                meter()->object(), meter()->property(), meter()->index());

    _confUi->lineEditDigits->setValidator(
                new QIntValidator(0, 100, _confUi->lineEditDigits));
    _confUi->lineEditDigits->setText(QString::number(meter()->digits()));

    connect(_buttonBox, &QDialogButtonBox::clicked, this, &MeterMenuHandler::confApply);

    connect(_confUi->dataSource, &DataSourceWidget::dataSourceChanged, this, &MeterMenuHandler::confChanged);
    connect(_confUi->lineEditDigits, &QLineEdit::textEdited, this, &MeterMenuHandler::confChanged);

    _confDialog->exec();

    delete _confDialog; _confDialog = 0;
    delete _confUi; _confUi = 0;
}

void MeterMenuHandler::confApply(QAbstractButton *button)
{
    if (_buttonBox->button(QDialogButtonBox::Apply) != button
	&& _buttonBox->button(QDialogButtonBox::Ok) != button) {
	return;
    }

    Q_ASSERT(_confUi && _confDialog);

    // XXX: check for actual change ?
    if(!_confChanged) return;
    _worldModel->beginMacro(i18n("Edit properties of %1", meter()->name()));

    _worldModel->setProperty(meter(), QStringLiteral("object"),
                        QVariant::fromValue(_confUi->dataSource->dataObject()));
    _worldModel->setProperty(meter(), QStringLiteral("property"),
                        _confUi->dataSource->dataProperty());
    _worldModel->setProperty(meter(), QStringLiteral("index"),
                        _confUi->dataSource->dataIndex());

    _worldModel->setProperty(meter(), QStringLiteral("digits"),
                        _confUi->lineEditDigits->text().toInt());

    _worldModel->endMacro();
}

void MeterMenuHandler::confChanged()
{
    Q_ASSERT(_confUi && _confDialog);
    _confChanged = true;
    _buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

////////////////////////////////////////////////////
ControllerGraphicsItem::ControllerGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WidgetGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Controller*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);
    setBackgroundBrush(QBrush(Qt::white));

    _widget = new QWidget();
    _widget->setPalette(QPalette(Qt::lightGray));
    QGridLayout* layout = new QGridLayout(_widget);

    _labelMin = new QLabel(_widget); _labelMin->setAlignment(Qt::AlignRight);
    _labelMax = new QLabel(_widget); _labelMax->setAlignment(Qt::AlignLeft);
    _labelSource = new QLabel(_widget); _labelSource->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    _slider = new QSlider(Qt::Horizontal, _widget);
    _slider->setRange(SLIDER_MIN, SLIDER_MAX);
    connect(_slider, &QAbstractSlider::sliderMoved, this, &ControllerGraphicsItem::sliderChanged);
    connect(_slider, &QAbstractSlider::sliderReleased, this, &ControllerGraphicsItem::sliderReleased);

    layout->addWidget(_labelMin, 0, 0, 1, 1);
    layout->addWidget(_slider, 0, 1, 1, 1);
    layout->addWidget(_labelMax, 0, 2, 1, 1);
    layout->addWidget(_labelSource, 1, 0, 1, 3);

    _incAction = new QAction(i18n("Increase value"), _widget);
    _decAction = new QAction(i18n("Decrease value"), _widget);

    connect(_incAction, &QAction::triggered, this, &ControllerGraphicsItem::incTriggered);
    connect(_decAction, &QAction::triggered, this, &ControllerGraphicsItem::decTriggered);

    _widget->addAction(_incAction);
    _widget->addAction(_decAction);
    //_widget->addAction(_configureAction);
    //_widget->setContextMenuPolicy(Qt::ActionsContextMenu);

    _lastValue = 1;
    _changed = false;

    setCenteralWidget(_widget);
    setOnHoverHandlerEnabled(true);
    _widget->setMouseTracking(true);
    _labelMin->setMouseTracking(true);
    _labelMax->setMouseTracking(true);
    _labelSource->setMouseTracking(true);
    _slider->setMouseTracking(true);
}

inline StepCore::Controller* ControllerGraphicsItem::controller() const
{
    return static_cast<StepCore::Controller*>(_item);
}

void ControllerGraphicsItem::decTriggered()
{
    _worldModel->simulationPause();
    _worldModel->beginMacro(i18n("Decrease controller %1", _item->name()));
    _worldModel->setProperty(controller(), QStringLiteral("value"),
                    controller()->value() - controller()->increment());
    _worldModel->endMacro();
}

void ControllerGraphicsItem::incTriggered()
{
    _worldModel->simulationPause();
    _worldModel->beginMacro(i18n("Increase controller %1", _item->name()));
    _worldModel->setProperty(controller(), QStringLiteral("value"),
                    controller()->value() + controller()->increment());
    _worldModel->endMacro();
}

void ControllerGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        viewScaleChanged();

        // Labels
        _labelMin->setText(QString::number(controller()->limits()[0]));
        _labelMax->setText(QString::number(controller()->limits()[1]));

        QString source;
        if(controller()->isValid()) {
            source = i18n("%1.%2", _worldModel->formatName(controller()->object()), controller()->property());
            if(controller()->index() >= 0) source.append(i18n("[%1]", controller()->index()));
            QString units = controller()->units();
            if(!units.isEmpty()) source.append(" [").append(units).append("]");
        } else {
            source = i18n("[not configured]");
        }
        _labelSource->setText(source);

        if(_incAction->isEnabled() != controller()->isValid()) {
            _incAction->setEnabled(controller()->isValid());
            _decAction->setEnabled(controller()->isValid());
        }

        if(_incShortcut != controller()->increaseShortcut()) {
            _incShortcut = controller()->increaseShortcut();
            _incAction->setShortcut(QKeySequence(_incShortcut));
        }

        if(_decShortcut != controller()->decreaseShortcut()) {
            _decShortcut = controller()->decreaseShortcut();
            _decAction->setShortcut(QKeySequence(_decShortcut));
        }

        //if(!graph()->autoLimitsX() && !graph()->autoLimitsY()) adjustLimits();

        /*
        // Points
        _plotObject->clearPoints();
        for(int i=0; i<(int) graph()->points().size(); ++i) {
            StepCore::Vector2d p = graph()->points()[i];
            _plotObject->addPoint(p[0], p[1]);
        }

        adjustLimits();
        _plotWidget->update();
        */

        WidgetGraphicsItem::worldDataChanged(dynamicOnly);
    }

    double value = round((controller()->value() - controller()->limits()[0]) *
            (SLIDER_MAX - SLIDER_MIN) / (controller()->limits()[1] - controller()->limits()[0]) + SLIDER_MIN);

    if(value <= SLIDER_MIN && _lastValue > SLIDER_MIN) {
        QPalette palette; palette.setColor(_labelMin->foregroundRole(), Qt::red);
        _labelMin->setPalette(palette);
    } else if(value > SLIDER_MIN && _lastValue <= SLIDER_MIN) {
        QPalette palette; _labelMin->setPalette(palette);
    }

    if(value >= SLIDER_MAX-1 && _lastValue < SLIDER_MAX-1) {
        QPalette palette; palette.setColor(_labelMax->foregroundRole(), Qt::red);
        _labelMax->setPalette(palette);
    } else if(value < SLIDER_MAX-1 && _lastValue >= SLIDER_MAX-1) {
        QPalette palette; _labelMax->setPalette(palette);
    }

    _lastValue = value;

    if(value < SLIDER_MIN) value = SLIDER_MIN;
    else if(value > SLIDER_MAX-1) value = SLIDER_MAX-1;

    _slider->setValue(int(value));

#if 0
    
    if(_worldModel->isSimulationActive()) {
        if(_worldModel->world()->time() > _lastPointTime
                    + 1.0/_worldModel->simulationFps() - 1e-2/_worldModel->simulationFps()) {
            StepCore::Vector2d point = graph()->recordPoint();
            _lastPointTime = _worldModel->world()->time();
        }
    }

    int po_count, p_count;
    do {
        const QList<KPlotPoint*> points = _plotObject->points();
        po_count = points.count(); p_count = graph()->points().size();
        int count = qMin(po_count, p_count);
        for(int p=0; p < count; ++p)
            points[p]->setPosition(vectorToPoint(graph()->points()[p]));
    } while(0);

    if(po_count < p_count) {
        for(; po_count < p_count; ++po_count)
            _plotObject->addPoint(vectorToPoint(graph()->points()[po_count]));
    } else {
        for(--po_count; po_count >= p_count; --po_count)
            _plotObject->removePoint(po_count);
    }

    if(graph()->autoLimitsX() || graph()->autoLimitsY()) adjustLimits();
    _plotWidget->update();
#endif
#if 0
//#error Do setProperty here and remove DynamicOnly from points
        if(ok) {
            _plotObject->addPoint(point[0], point[1]);
            if(graph()->autoLimitsX() || graph()->autoLimitsY()) 
                adjustLimits();
            _plotWidget->update();
        }
        _lastPointTime = _worldModel->world()->time();
        worldDataChanged(false);
        //_worldModel->setProperty(graph(), "name", QString("test"));
    }
#endif
}

void ControllerGraphicsItem::sliderChanged(int value)
{
    Q_ASSERT(value == _slider->sliderPosition());
    if(!controller()->isValid()) return;
    //if(!_worldModel->isSimulationActive()) {
        _worldModel->simulationPause();
        if(!_changed) {
            _worldModel->beginMacro(i18n("Change controller %1", controller()->name()));
            _changed = true;
        }
        double v = controller()->limits()[0] + (value - SLIDER_MIN) *
                (controller()->limits()[1] - controller()->limits()[0]) / (SLIDER_MAX - SLIDER_MIN);
        _worldModel->setProperty(controller(), QStringLiteral("value"), v);
    //}
}

void ControllerGraphicsItem::sliderReleased()
{
    if(_changed) {
        _worldModel->endMacro();
        _changed = false;
    }
}

void ControllerMenuHandler::populateMenu(QMenu* menu, KActionCollection* actions)
{
    _confUi = 0;
    _confDialog = 0;
    _confChanged = false;

    menu->addAction(QIcon::fromTheme(QStringLiteral("arrow-up")), i18n("Increase value"), this, &ControllerMenuHandler::incTriggered);
    menu->addAction(QIcon::fromTheme(QStringLiteral("arrow-down")), i18n("Decrease value"), this, &ControllerMenuHandler::decTriggered);
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure controller..."), this, &ControllerMenuHandler::configureController);
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu, actions);
}

inline StepCore::Controller* ControllerMenuHandler::controller() const
{
    return static_cast<StepCore::Controller*>(_object);
}

void ControllerMenuHandler::configureController()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _confChanged = false;
    _confDialog = new QDialog(); // XXX
    
    _confDialog->setWindowTitle(i18n("Configure controller"));
    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
				      | QDialogButtonBox::Cancel
				      | QDialogButtonBox::Apply);
    QWidget *mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    _confDialog->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    QPushButton *okButton = _buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    _confDialog->connect(_buttonBox, &QDialogButtonBox::accepted, _confDialog, &QDialog::accept);
    _confDialog->connect(_buttonBox, &QDialogButtonBox::rejected, _confDialog, &QDialog::reject);
    mainLayout->addWidget(_buttonBox);

    _confUi = new Ui::WidgetConfigureController;
    _confUi->setupUi(mainWidget);

    _confUi->dataSource->setSkipReadOnly(true);
    _confUi->dataSource->setDataSource(_worldModel, controller()->object(),
                                    controller()->property(), controller()->index());

    _confUi->lineEditMin->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMin));
    _confUi->lineEditMax->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMax));

    _confUi->lineEditMin->setText(QString::number(controller()->limits()[0]));
    _confUi->lineEditMax->setText(QString::number(controller()->limits()[1]));

    _confUi->keyIncrease->setModifierlessAllowed(true);
    _confUi->keyDecrease->setModifierlessAllowed(true);

    //_confUi->keyIncrease->setKeySequence(_incAction->shortcut().primary());
    //_confUi->keyDecrease->setKeySequence(_decAction->shortcut().primary());
    _confUi->keyIncrease->setKeySequence(QKeySequence(controller()->increaseShortcut()));
    _confUi->keyDecrease->setKeySequence(QKeySequence(controller()->decreaseShortcut()));

    _confUi->lineEditIncrement->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditIncrement));
    _confUi->lineEditIncrement->setText(QString::number(controller()->increment()));

    _buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    connect(_buttonBox, &QDialogButtonBox::clicked, this, &ControllerMenuHandler::confApply);

    connect(_confUi->dataSource, &DataSourceWidget::dataSourceChanged, this, &ControllerMenuHandler::confChanged);
    connect(_confUi->lineEditMin, &QLineEdit::textEdited, this, &ControllerMenuHandler::confChanged);
    connect(_confUi->lineEditMax, &QLineEdit::textEdited, this, &ControllerMenuHandler::confChanged);
    connect(_confUi->keyIncrease, &KKeySequenceWidget::keySequenceChanged, this, &ControllerMenuHandler::confChanged);
    connect(_confUi->keyDecrease, &KKeySequenceWidget::keySequenceChanged, this, &ControllerMenuHandler::confChanged);
    connect(_confUi->lineEditIncrement, &QLineEdit::textEdited, this, &ControllerMenuHandler::confChanged);

    _confDialog->exec();

    delete _confDialog; _confDialog = 0;
    delete _confUi; _confUi = 0;
}

void ControllerMenuHandler::confApply(QAbstractButton *button)
{
    if (_buttonBox->button(QDialogButtonBox::Apply) != button
	&& _buttonBox->button(QDialogButtonBox::Ok) != button) {
	return;
    }

    Q_ASSERT(_confUi && _confDialog);

    // XXX: check for actual change ?
    if(!_confChanged) return;
    _worldModel->beginMacro(i18n("Edit properties of %1", controller()->name()));

    _worldModel->setProperty(controller(), QStringLiteral("object"),
                    QVariant::fromValue(_confUi->dataSource->dataObject()));
    _worldModel->setProperty(controller(), QStringLiteral("property"),
                    _confUi->dataSource->dataProperty());
    _worldModel->setProperty(controller(), QStringLiteral("index"),
                    _confUi->dataSource->dataIndex());

    StepCore::Vector2d limits(_confUi->lineEditMin->text().toDouble(),
                              _confUi->lineEditMax->text().toDouble());

    _worldModel->setProperty(controller(), QStringLiteral("limits"),
                            QVariant::fromValue(limits));

    _worldModel->setProperty(controller(), QStringLiteral("increaseShortcut"),
                            QVariant::fromValue(_confUi->keyIncrease->keySequence().toString()));

    _worldModel->setProperty(controller(), QStringLiteral("decreaseShortcut"),
                            QVariant::fromValue(_confUi->keyDecrease->keySequence().toString()));

    _worldModel->setProperty(controller(), QStringLiteral("increment"),
                            QVariant::fromValue(_confUi->lineEditIncrement->text().toDouble()));

    _worldModel->endMacro();
}

void ControllerMenuHandler::confChanged()
{
    Q_ASSERT(_confUi && _confDialog);
    _confChanged = true;
    _buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ControllerMenuHandler::decTriggered()
{
    _worldModel->simulationPause();
    _worldModel->beginMacro(i18n("Decrease controller %1", _object->name()));
    _worldModel->setProperty(controller(), QStringLiteral("value"),
                    controller()->value() - controller()->increment());
    _worldModel->endMacro();
}

void ControllerMenuHandler::incTriggered()
{
    _worldModel->simulationPause();
    _worldModel->beginMacro(i18n("Increase controller %1", _object->name()));
    _worldModel->setProperty(controller(), QStringLiteral("value"),
                    controller()->value() + controller()->increment());
    _worldModel->endMacro();
}

////////////////////////////////////////////////////

TracerGraphicsItem::TracerGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : StepGraphicsItem(item, worldModel), _moving(false), _movingDelta(0,0)
{
    Q_ASSERT(dynamic_cast<StepCore::Tracer*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);

    /*
    _lastArrowRadius = -1;
    _velocityHandler = new ArrowHandlerGraphicsItem(item, worldModel, this,
                   _item->metaObject()->property("velocity"));
    _velocityHandler->setVisible(false);*/
    //scene()->addItem(_velocityHandler);

    _lastPos = QPointF(0,0);
    _lastPointTime = -HUGE_VAL;
}

inline StepCore::Tracer* TracerGraphicsItem::tracer() const
{
    return static_cast<StepCore::Tracer*>(_item);
}


QPainterPath TracerGraphicsItem::shape() const
{
    QPainterPath path;
    double w = (HANDLER_SIZE+1)/currentViewScale();
    // XXX: add _points here!
    path.addEllipse(QRectF(_lastPos.x()-w,  _lastPos.y()-w,w*2,w*2));
    return path;
}

void TracerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    double s = currentViewScale();
    double w = HANDLER_SIZE/s;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor::fromRgba(tracer()->color()), 0));
    //painter->setBrush(QBrush(Qt::black));
    painter->drawPolyline(_points);
    painter->drawEllipse(QRectF(_lastPos.x()-w,  _lastPos.y()-w, w*2,w*2));
    painter->drawPoint(_lastPos);

    if(_isSelected) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        //painter->setBrush(QBrush());
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        w = (HANDLER_SIZE + SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(_lastPos.x()-w, _lastPos.y()-w, w*2, w*2));
    }

}

void TracerGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();

    double s = currentViewScale();
    double w = (HANDLER_SIZE+SELECTION_MARGIN)/s;
    QPointF p = vectorToPoint(tracer()->position());

    _boundingRect = _points.boundingRect() | QRectF(p.x()-w, p.y()-w,2*w,2*w);
    update();
}

void TracerGraphicsItem::worldDataChanged(bool)
{
    /*
    if(_isMouseOverItem || _isSelected) {
        double vnorm = particle()->velocity().norm();
        double anorm = particle()->force().norm() / particle()->mass();
        double arrowRadius = qMax(vnorm, anorm) + ARROW_STROKE/currentViewScale();
        if(arrowRadius > _lastArrowRadius || arrowRadius < _lastArrowRadius/2) {
            _lastArrowRadius = arrowRadius;
            viewScaleChanged();
        }
        update();
    }
    */

    //setPos(vectorToPoint(tracer()->position()));

    if(_worldModel->isSimulationActive()) {
        if(_worldModel->world()->time() > _lastPointTime
                    + 1.0/_worldModel->simulationFps() - 1e-2/_worldModel->simulationFps()) {
            tracer()->recordPoint();
            _lastPointTime = _worldModel->world()->time();
        }
    }

    bool geometryChange = false;
    int po_count, p_count;
    do {
        po_count = _points.size(); p_count = tracer()->points().size();
        int count = qMin(po_count, p_count);
        for(int p=0; p < count; ++p) {
            QPointF point = vectorToPoint(tracer()->points()[p]);
            if(point != _points[p]) {
                geometryChange = true;
                _points[p] = point;
            }
        }
    } while(0);

    if(po_count < p_count) {
        geometryChange = true;
        for(; po_count < p_count; ++po_count)
            _points << vectorToPoint(tracer()->points()[po_count]);
    } else {
        geometryChange = true;
        _points.resize(p_count);
    }

    QPointF point = vectorToPoint(tracer()->position());
    if(point != _lastPos) {
        geometryChange = true;
        _lastPos = point;
    }

    if(geometryChange) {
        viewScaleChanged();
    }
}

void TracerGraphicsItem::mouseSetPos(const QPointF&, const QPointF& diff, MovingState movingState)
{
    static_cast<WorldScene*>(scene())->snapItem(vectorToPoint(tracer()->position()) + diff,
                WorldScene::SnapRigidBody | WorldScene::SnapParticle |
                WorldScene::SnapSetLocalPosition, 0, movingState, _item);
}

void TracerMenuHandler::populateMenu(QMenu* menu, KActionCollection* actions)
{
    menu->addAction(QIcon::fromTheme(QStringLiteral("edit-clear")), i18n("Clear trace"), this, &TracerMenuHandler::clearTracer);
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu, actions);
}

void TracerMenuHandler::clearTracer()
{
    _worldModel->simulationPause();
    //_lastPointTime = -HUGE_VAL; // XX
    _worldModel->beginMacro(i18n("Clear tracer %1", _object->name()));
    _worldModel->setProperty(_object, QStringLiteral("points"),
                   QVariant::fromValue(StepCore::Vector2dList()) );
    _worldModel->endMacro();
}

