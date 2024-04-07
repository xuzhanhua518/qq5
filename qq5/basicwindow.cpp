#include "basicwindow.h"
#include "CommonUtils.h"
#include "NotifyManager.h"

#include <QFile>
#include <QStyleOption>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QDesktopWidget>


BasicWindow::BasicWindow(QWidget *parent)
	: QDialog(parent)
{
	m_colorBackGround = CommonUtils::getDefaultSkinColor();
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	connect(NotifyManager::getInstance(), SIGNAL(onSignalSkinChanged(const QColor&)), this, SLOT(onSignalSkinChanged(const QColor&)));
}

BasicWindow::~BasicWindow()
{

}

void BasicWindow::loadStyleSheet(const QString& sheetName)
{
	m_styleName = sheetName;
	QFile file(":/Resources/QSS/" + m_styleName + ".css");
	file.open(QFile::ReadOnly);

	if (file.isOpen())
	{
		setStyleSheet("");
		QString qsstyleSheet = QLatin1String(file.readAll());			// 数据内容全部读取出来, 保存到 qstyleSheet字符串里

		// 获取当前用户的皮肤RGB值
		QString r = QString::number(m_colorBackGround.red());
		QString g = QString::number(m_colorBackGround.green());
		QString b = QString::number(m_colorBackGround.blue());

		// titleskin 标题皮肤属性为 true
		// 底部的皮肤 bottomskin 为 true
		// rgba 的 a ,是透明度
		qsstyleSheet += QString("QWidget[titleskin=true]\
												{background-color:rgb(%1,%2,%3);\
												border-top-left-radius:4px;}\
												QWidget[bottomskin=true]\
												{border-top:1px solid rgba(%1,%2,%3,100);\
												background-color:rgba(%1,%2,%3,50);\
												border-bottom-left-radius:4px;\
												border-bottom-right-radius:4px;}")
												.arg(r).arg(g).arg(b);
		// 设置样式表
		setStyleSheet(qsstyleSheet);
	}

	file.close();

}

//Í·Ïñ×ªÔ²Í·Ïñ
QPixmap BasicWindow::getRoundImage(const QPixmap & src, QPixmap & mask, QSize maskSize)
{
	if (maskSize == QSize(0, 0))
	{
		maskSize = mask.size();
	}
	else
	{	
		// Qt::KeepAspectRatio , 缩放时, 尽可能以大的矩形缩放
		// Qt::SmoothTransformation , 平滑方式
		mask = mask.scaled(maskSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	// 保存转换后的头像
	// QImage::Format_ARGB32_Premultiplied 格式, 获取的头像会比较清晰
	QImage resultImage(maskSize, QImage::Format_ARGB32_Premultiplied);

	// CompositionMode , 图片的叠加模式
	QPainter painter(&resultImage);				// 定义一个画家, 画 resultImage
	painter.setCompositionMode(QPainter::CompositionMode_Source);			// 设置图片叠加模式, 将源文件以 复制进行操作 ， Source 将原图片，直接拷贝过来
	painter.fillRect(resultImage.rect(), Qt::transparent);										// 填充矩形, 矩形的大小, 就是图片的大小 , 同时是透明的 transparent
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);	// 用SourceOver方式, 进行叠加
	painter.drawPixmap(0, 0, mask);																		// 对空白的圆形图片 , 进行叠加
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);		// 用SourceIn方式, 进行叠加
	painter.drawPixmap(0, 0, src.scaled(maskSize,Qt::KeepAspectRatio, Qt::SmoothTransformation));		// 在对目标的QQ头像, 进行比例设置 
	painter.end();

	return QPixmap::fromImage(resultImage);
}


void BasicWindow::onShowClose(bool)
{
	close();
}

//±³¾°Í¼
void BasicWindow::initBackGroundColor()
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

//×ÓÀà»¯²¿¼þÊ±£¬ÐèÒªÖØÐ´»æÍ¼ÊÂ¼þÉèÖÃ±³¾°Í¼
void BasicWindow::paintEvent(QPaintEvent *event)
{
	initBackGroundColor();
	QDialog::paintEvent(event);
}

void BasicWindow::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		// pos() , 事件发生时, 相对于窗口左上角的 偏移坐标
		m_mousePressed = true;
		m_mousePoint = event->globalPos() - pos();		// 减去 坐标值
		event->accept();			// 接受
	}
}

//鼠标移动事件
void BasicWindow::mouseMoveEvent(QMouseEvent * event)
{
	if (m_mousePressed && (event->buttons() && Qt::LeftButton))
	{
		//event->globalPos()  事件发生时全局坐标，相对于屏幕左上角（0,0）
		move(event->globalPos() - m_mousePoint);
		event->accept();
	}
}

void BasicWindow::mouseReleaseEvent(QMouseEvent * event)
{
	m_mousePressed = false;

}



void BasicWindow::onSignalSkinChanged(const QColor & color)
{
	m_colorBackGround = color;
	loadStyleSheet(m_styleName);
}

void BasicWindow::onButtonMinClicked()
{
	if (Qt::Tool == (windowFlags() & Qt::Tool))
	{
		hide();
	}
	else
	{
		showMinimized();
	}
}

void BasicWindow::onButtonRestoreClicked()
{
	QPoint windowPos;
	QSize windowSize;
	_titleBar->getRestoreInfo(windowPos, windowSize);
	setGeometry(QRect(windowPos, windowSize));
}

void BasicWindow::onButtonMaxClicked()
{
	_titleBar->saveRestoreInfo(pos(), QSize(width(), height()));
	QRect desktopRect = QApplication::desktop()->availableGeometry();
	QRect factRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3, desktopRect.width() + 6, desktopRect.height() + 6);
	setGeometry(factRect);
}

void BasicWindow::onButtonCloseClicked()
{
	close();
}

void BasicWindow::initTitleBar(ButtonType buttontype)
{
	_titleBar = new TitleBar(this);
	_titleBar->setButtonType(buttontype);
	_titleBar->move(0, 0);
	connect(_titleBar, SIGNAL(signalButtonMinClicked()), this, SLOT(onButtonMinClicked()));
	connect(_titleBar, SIGNAL(signalButtonRestoreClicked()), this, SLOT(onButtonRestoreClicked()));
	connect(_titleBar, SIGNAL(signalButtonMaxClicked()), this, SLOT(onButtonMaxClicked()));
	connect(_titleBar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));
}

void BasicWindow::setTitleBarTitle(const QString & title, const QString & icon)
{
	_titleBar->setTitleIcon(icon);
	_titleBar->setTitleContent(title);
}

void BasicWindow::onShowMin(bool)
{
	showMinimized();
}

void BasicWindow::onShowHide(bool)
{
	hide();
}

void BasicWindow::onShowNormal(bool)
{
	show();
	activateWindow();
}

void BasicWindow::onShowQuit(bool)
{
	QApplication::quit();
}
