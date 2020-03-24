#include <QFileDialog>
#include <basic_surf_objs.h>
#include <QAxObject>
#include <v3d_interface.h>
#include <iostream>
#include <QDebug>


using namespace std;

void exportGrayValue(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString dirName = QFileDialog ::getExistingDirectory(parent, "Select images");
    QDir dir(dirName);
    QStringList filters;
    filters << "*.tif"<<"*.v3draw";
    QFileInfoList imageName = dir.entryInfoList(filters, QDir::Files);
    QString excelName = dirName + "\\GrayValue.xlsx";

    cout<<"excelName: "<<excelName.toStdString()<<endl;

    unsigned char * data1d0 = 0;
    V3DLONG in_sz0[4]={0};
    int datatype0;
    QString file = imageName[0].filePath();
    qDebug()<<__LINE__;
    if(!simple_loadimage_wrapper(callback, file.toStdString().c_str(), data1d0, in_sz0, datatype0))
    {
        qDebug()<< "Error happens in reading the subject file.";
        return;
    }

    qDebug()<<__LINE__;

    QAxObject *excel = new QAxObject(parent);
    excel -> setControl("Excel.Application"); // connect EXCEL control
    excel -> setProperty("DisplayAlerts", true); // display window
    QAxObject *workbooks = excel -> querySubObject("WorkBooks"); // obtain excel set
    workbooks -> dynamicCall("Add"); // create a new excel
    QAxObject *workbook = excel -> querySubObject("ActiveWorkBook"); // obtain current excel
    workbook -> dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool)",
                            excelName, 51, QString(""), QString(""), false, false);
    //51xlsx, 56xls
    QAxObject *worksheets = workbook->querySubObject("WorkSheets"); // obtain sheets set
    QAxObject *worksheet1 = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
    QAxObject *worksheet2 = worksheets -> querySubObject("Add()"); // create a new sheet

    qDebug()<<__LINE__;

     V3DLONG ROW_NUM {in_sz0[1]+1};
     V3DLONG COL_NUM {imageName.size()+1};

     vector<unsigned char *> data1d_vec(imageName.size());
     vector<V3DLONG *> in_sz_vec(imageName.size());
     vector<int> datatype_vec(imageName.size());
     vector<unsigned short *> data1d_copy(imageName.size());
     for(V3DLONG i = 0; i < imageName.size(); i++)
     {
         in_sz_vec[i] = new V3DLONG[4];
         qDebug()<<__LINE__;
         if(!simple_loadimage_wrapper(callback, imageName[i].filePath().toStdString().c_str(), data1d_vec[i], in_sz_vec[i], datatype_vec[i]))
         {
             qDebug()<< "Error happens in reading the subject file.";
             return ;
         }

         qDebug()<<__LINE__;
         data1d_copy[i] = new unsigned short[in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3] *datatype_vec[i]/2];
         memcpy(data1d_copy[i], data1d_vec[i], in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3]*datatype_vec[i]);
     }

     qDebug()<<__LINE__;





     QList <QList<QVariant>> datas_sheet1;
     QList <QList<QVariant>> datas_sheet2;
     for(V3DLONG i = 0; i < ROW_NUM; i++)
     {
        QList<QVariant> rows1, rows2;
        if(i == 0)
        {
            rows1.append("");
            rows2.append("");
            for(V3DLONG j = 0; j < COL_NUM-1; j ++)
            {
                rows1.append(imageName[j].baseName());
                rows2.append(imageName[j].baseName());
            }

        }
        else
        {
            rows1.append(i);
            rows2.append(i);
            for(V3DLONG j = 0; j < COL_NUM-1; j ++)
            {

                rows1.append(data1d_copy[j][(i-1)*in_sz_vec[j][0]+in_sz_vec[j][0]/2-1]);
                rows2.append(data1d_copy[j][(i-1)*in_sz_vec[j][0]+in_sz_vec[j][0]/2]);
            }
        }
        datas_sheet1.append(rows1);
        datas_sheet2.append(rows2);
     }

     qDebug()<<__LINE__;

     // Convert 2D to 1D
     QList<QVariant> vars1, vars2;
     for(auto v:datas_sheet1)
     {
         vars1.append(QVariant(v));
     }
     for(auto v:datas_sheet2)
     {
         vars2.append(QVariant(v));
     }
     QVariant var1 = QVariant(vars1);
     QVariant var2 = QVariant(vars2);

     char a = (char)(imageName.size()+48+1+16);
     cout<<"a_char = "<<a<<endl;
     string s(1, a);
     QString qs = QString ::fromStdString(s);
     QString num = QString::fromStdString(to_string(ROW_NUM));
     QString range = "A1:"+qs+num;
     cout<<"excel range: "<<range.toStdString()<<endl;

     QAxObject *excel_property1 = worksheet1 -> querySubObject("Range(const QString&)", range);
     qDebug()<<__LINE__;
     QAxObject *excel_property2 = worksheet2 -> querySubObject("Range(const QString&)", range);
     qDebug()<<__LINE__;

     excel_property1->setProperty("Value", var1);
     excel_property1->setProperty("HorizontalAlignment", -4108);
     qDebug()<<__LINE__;
     excel_property2->setProperty("Value", var2);
     excel_property2->setProperty("HorizontalAlignment", -4108);
     // x1Left: -4130; x1Center: -4108; x1Right: -4152

     qDebug()<<__LINE__;

     workbook->dynamicCall("Save()");
     workbook->dynamicCall("Close(Boolean)", false);
     excel->dynamicCall("Quit(void)");
     delete excel;

}
