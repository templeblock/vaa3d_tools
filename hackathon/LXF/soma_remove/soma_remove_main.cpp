#include "soma_remove_main.h"

#include <iostream>
#include "string.h"
 using namespace std;
//#define DIS((x,y,z),b) (x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)+(z-b.z)*(z-b.z)
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
//#define NTDISs(a,b) (sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z)))
bool FAR = 0;
bool CLOSE = 1;
int dis_thresh = 3;

vector<bool> classify_glio(Chart &chart1,Each_line &E1,Chart &chart2,Each_line &E2,Chart &chart_curr,Each_line &E_curr,Feature &feature_curr,bool &method);
bool export_training_data(const QString &fileOpenName,Chart &chart,Each_line &E);
//bool if_is_connect(Coordinate* &curr,Coordinate* &b,vector<vector<vector<V3DLONG> > > &mark3D);
uint qHash(const Coordinate key)
{
    return key.x + key.y + key.z;
}
uint qHash(const vector<Coordinate> key)
{
    return key[0].x + key[0].y + key[0].z;
}

bool soma_remove_main(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback)
{
    V3DLONG M = in_sz[0];
    V3DLONG N = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG C = in_sz[3];
    int thresh = 30;                            //need to be change
    double signal_thresh = 4;                        //need to be change
    double r = 5;                                  //need to be change
    V3DLONG im_cropped_sz[4];
    unsigned char *im_cropped = 0;


    double d = 2*r;
    int y=0;
    im_cropped_sz[0] = in_sz[0];
    im_cropped_sz[1] = in_sz[1];
    im_cropped_sz[2] = in_sz[2];
    im_cropped_sz[3] = in_sz[3];
    V3DLONG pagesz;
    //cout<<im_cropped_sz[0]<<"   "<<im_cropped_sz[1]<<"    "<<im_cropped_sz[2]<<"    "<<im_cropped_sz[3]<<endl;
    pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
     cout<<"pagesz = "<<pagesz<<endl;
    try {im_cropped = new unsigned char [pagesz];}
     catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

     vector<vector<vector<V3DLONG> > > coodinate3D;
     vector<vector<V3DLONG> > coodinate2D;
     vector<V3DLONG> coodinate1D;

     for(V3DLONG iz = 0; iz < P; iz++)
     {
         V3DLONG offsetk = iz*M*N;
         for(V3DLONG iy = 0; iy < N; iy++)
         {
             V3DLONG offsetj = iy*N;
             for(V3DLONG ix = 0; ix < M; ix++)
             {
                 V3DLONG tmp = data1d[offsetk + offsetj + ix];
                 if(tmp==0)y++;
                 coodinate1D.push_back(tmp);
             }
             coodinate2D.push_back(coodinate1D);
             coodinate1D.clear();
         }
         coodinate3D.push_back(coodinate2D);
         coodinate2D.clear();
     }
     for(V3DLONG iz = 1; iz < P-1; iz++)
     {
         for(V3DLONG iy = 1; iy < N-1; iy++)
         {
             for(V3DLONG ix = 1; ix < M-1; ix++)
             {
                 int bri = coodinate3D[iz][iy][ix];

                 //cout<<"bri = "<<bri<<endl;
                 if(bri>thresh)
                 {

                     //NI = coodinate3D[iz][iy][ix-1]
                     int SI = coodinate3D[iz][iy][ix+1];
                     int EI = coodinate3D[iz][iy-1][ix];
                     int WI = coodinate3D[iz][iy+1][ix];
                     //DI = coodinate3D[iz-1][iy][ix]
                     //JI = coodinate3D[iz+1][iy][ix]
                     if((SI - bri)*(SI - bri)< signal_thresh)
                     {
                         double count_1=0;
                         for(int ixi=0;ixi<r;ixi++)
                         {
                             for(int iyi=0;iyi<r;iyi++)
                             {
                                 if(ix+ixi>M-1)continue;
                                 if(iy+iyi>N-1)continue;
                                 if(coodinate3D[iz][iy+iyi][ix+ixi]>thresh)
                                 {
                                     count_1++;
                                 }
                             }
                         }

                         double per_1 = (r*r-count_1)/(r*r);
                         //cout<<"per_1 = "<<per_1<<endl;
                         if(per_1>0.7)
                         {
                             for(int k=0;k<d;k++)
                             {
                                 for(int l=0;l<d;l++)
                                 {
                                     cout<<"33333333333333"<<endl;
                                     if(ix+l>M-1)continue;
                                     if(iy+k>N-1)continue;
                                     coodinate3D[iz][iy+k][ix+l]=0;
                                 }
                             }
                         }


                     }
                     if((WI - bri)*(WI - bri)< signal_thresh)
                     {
                         double count_2=0;
                         for(int ixi=0;ixi<r;ixi++)
                         {
                             for(int iyi=0;iyi<r;iyi++)
                             {
                                 if(ix-r/2+ixi>M-1)continue;
                                 if(iy+iyi>N-1)continue;
                                 if(coodinate3D[iz][iy+iyi][ix-r/2+ixi]>thresh)
                                 {
                                     count_2++;
                                 }
                             }
                         }
                         double per_2 = (r*r-count_2)/(r*r);
                         //cout<<"per_2 = "<<per_2<<endl;
                         if(per_2>0.7)
                         {
                             for(int k=0;k<d;k++)
                             {
                                 for(int l=0;l<d;l++)
                                 {
                                     if(ix-r/2+l>M-1)continue;
                                     if(iy+k>N-1)continue;
                                     cout<<"22222222222222"<<endl;
                                     coodinate3D[iz][iy+k][ix-r/2+l]=0;
                                 }
                             }
                         }
                     }
                     //                         if((EI - bri)*(EI -u bri)< signal_thresh)
                     //                         {
                     //                             double count_3=0;
                     //                             for(int ixi=0;ixi<r;ixi++)
                     //                             {
                     //                                 for(int iyi=0;iyi<r;iyi++)
                     //                                 {
                     //                                     if(coodinate3D[iz][iy+iyi][ix-r/2+ixi]>thresh)
                     //                                     {
                     //                                         count_3++;
                     //                                     }
                     //                                 }
                     //                             }
                     //                             double per_3 = (r*r-count_2)/(r*r);
                     //                             if(per_3>0.7)
                     //                             {

                     //                             }
                     //                         }

                 }
             }
         }
     }
     cout<<"hahhahaa"<<endl;
     //int u=0;
     for(V3DLONG iz = 0; iz < P; iz++)
     {
         V3DLONG offsetk = iz*M*N;
         for(V3DLONG iy = 0; iy < N; iy++)
         {
             V3DLONG offsetj = iy*N;
             for(V3DLONG ix = 0; ix < M; ix++)
             {
                 im_cropped[offsetk + offsetj + ix] = coodinate3D[iz][iy][ix];
                 //u++;
             }
         }
     }
     int u=0;
     for(V3DLONG iz = 0; iz < P; iz++)
     {
         for(V3DLONG iy = 0; iy < N; iy++)
         {
             for(V3DLONG ix = 0; ix < M; ix++)
             {
                 int o = coodinate3D[iz][iy][ix];
                 if(0==0)
                 {
                     u++;
                 }
             }
         }
     }
     cout<<u<<"  "<<y<<endl;
     v3d_msg("check");
         simple_saveimage_wrapper(callback,QString("tmpimg.v3draw").toStdString().c_str(),im_cropped,in_sz,1);




}
bool soma_remove_main_2(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback)
{
    bool first_floor = false;
    V3DLONG M = in_sz[0];
    V3DLONG N = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG C = in_sz[3];
    V3DLONG pagesz = M*N*P;
    cout<<"1.make binary image."<<endl;
    unsigned char *im_cropped = 0;
    try {im_cropped = new unsigned char [pagesz];}
     catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
    V3DLONG signal_all=0;
    for(V3DLONG i=0;i<pagesz;i++)
    {
        signal_all = signal_all + int(data1d[i]);
    }
    V3DLONG signal = signal_all/pagesz;
    cout<<"signal_all = "<<signal_all<<endl;
    cout<<"signal = "<<signal<<endl;
    for(V3DLONG i=0;i<pagesz;i++)
    {
        if(int(data1d[i]) < signal+10)
        {
            im_cropped[i] = 0;
        }
        else
        {
            im_cropped[i] = 255;
        }
    }
    simple_saveimage_wrapper(callback,QString("binary.v3draw").toStdString().c_str(),im_cropped,in_sz,1);
    cout<<"2.make connected area."<<endl;
    vector<vector<vector<V3DLONG> > > coodinate3D,mark3D;
    vector<vector<V3DLONG> > coodinate2D,mark2D;
    vector<V3DLONG> coodinate1D,mark1D;
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < N; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < M; ix++)
            {
                V3DLONG tmp = im_cropped[offsetk + offsetj + ix];
                ImageMarker coord_curr;
                if(tmp==255)
                {
                    int one=1;
                    mark1D.push_back(one);

                }
                else
                {
                    int zero=0;
                    mark1D.push_back(zero);
                }
                coodinate1D.push_back(tmp);
            }
            coodinate2D.push_back(coodinate1D);
            mark2D.push_back(mark1D);
            coodinate1D.clear();
            mark1D.clear();
        }
        coodinate3D.push_back(coodinate2D);
        mark3D.push_back(mark2D);
        coodinate2D.clear();
        mark2D.clear();
    }
    int j = 1;
    int u = 0;
    int g = 0;
    vector<vector<Coordinate> >connected_region;
    QHash<Coordinate,int> map_index;
    vector<Coordinate> each_region1;
    vector<Coordinate> each_region;
    QList<ImageMarker> marker_all3;
    int black = 0;
    int white = 0;
    int unknow = 0;
    int black2 = 0;
    int all = 0;
    for(int iz = 0; iz < P; iz++)
    {
        for(int iy = 0; iy < N; iy++)
        {
            for(int ix = 0; ix < M; ix++)
            {
                all++;
                Coordinate coord_curr;
                if(coodinate3D[iz][iy][ix]==255)
                {

                    if(!first_floor)
                    {
                        coord_curr.x = ix;
                        coord_curr.y = iy;
                        coord_curr.z = iz;
                        each_region1.push_back(coord_curr);
                        connected_region.push_back(each_region1);
                        map_index.insert(coord_curr,j);
                        each_region1.clear();
                        j++;
                        g++;
                        first_floor = true;
                        unknow++;
                        continue;
                    }
                }
                else
                {
                    black++;
                    continue;
                }
                if(first_floor)
                {
                    if(coodinate3D[iz][iy][ix]==255)
                    {
                        white++;
                        int dex;

                        bool ind1 = false;
                        coord_curr.x = ix;
                        coord_curr.y = iy;
                        coord_curr.z = iz;
                        ImageMarker m;
                        m.x = coord_curr.x;
                        m.y = coord_curr.y;
                        m.z = coord_curr.z;
                        m.radius = 0.5;
                        m.type = 2;
                        marker_all3.push_back(m);
                        for(int l=0;l<connected_region.size();l++)
                        {
                            for(int k=0;k<connected_region[l].size();k++)
                            {
                                if(if_is_connect(coord_curr,connected_region[l][k],mark3D))
                                {
                                    dex = l;
                                    ind1 = true;
                                    break;
                                }
                            }
                            if(ind1)
                            {
                                break;
                            }

                        }
                        if(ind1)
                        {
                            connected_region[dex].push_back(coord_curr);
                            map_index.insert(coord_curr,map_index[connected_region[dex][0]]);
                            ind1 = false;
                            //break;
                        }
                        else
                        {
                            each_region.clear();
                            each_region.push_back(coord_curr);
                            u++;
                            connected_region.push_back(each_region);
                            map_index.insert(coord_curr,j);
                            j++;

                        }
                        each_region.clear();

                    }
                    else
                    {
                        black2++;
                    }

                }
            }
        }

    }
    QHash<vector<Coordinate>,vector<Coordinate> > relation;
    int count_ind=1;
    //vector<int> count;
    vector<V3DLONG> index;
    vector<V3DLONG> reminder;
    for(int l=0;l<connected_region.size();l++)
    {
        int zero = 0;
        reminder.push_back(zero);
    }
    for(int l=0;l<connected_region.size();l++)
    {
        for(int j=l+1;j<connected_region.size();j++)
        {
            if(reminder[l+1]!=0)continue;
            double min_dis = 1000000000000;
            for(int k=0;k<connected_region[l].size();k++)
            {
                for(int h=0;h<connected_region[j].size();h++)
                {

                    double dis = NTDIS(connected_region[l][k],connected_region[j][h]);
                    if(dis<min_dis)
                    {
                        min_dis = dis;
                    }

                }
            }
            if(min_dis<dis_thresh)
            {
                relation.insert(connected_region[l],connected_region[j]);
                reminder[l] = 1;
                reminder[j] = 1;
                index.push_back(l);
                count_ind++;
            }
        }
    }
    int n=2;
    vector<vector<Coordinate> >connected_region_final;
    vector<V3DLONG> reminder2;
    for(int l=0;l<connected_region.size();l++)
    {
        int zero = 0;
        reminder2.push_back(zero);
    }
    for(int l=0;l<connected_region.size();l++)
    {
        if(reminder2[l]!=0)
        {
            continue;
        }
        if(relation[connected_region[l]].size()==0)
        {
            connected_region_final.push_back(connected_region[l]);
            reminder2[l]=1;
        }
        else
        {
            vector<Coordinate> tmp;
            for(int y=0;y<connected_region[l].size();y++)
            {
                tmp.push_back(connected_region[l][y]);
            }
            reminder2[l]=1;
            find_relation(relation,connected_region[l],tmp,reminder2,map_index);
            connected_region_final.push_back(tmp);
        }
    }

    cout<<"3.train_data"<<endl;
    Chart chart,chart2;
    Each_line E,E2;
    export_training_data(QString("signal.txt"),chart,E);
    export_training_data(QString("glio.txt"),chart2,E2);




    cout<<"4.calculate feature."<<endl;


    vector<double> y_n;
    vector<double> overlap_level;
    vector<double> ratio_v;
    vector<double> count_v;
    vector<double> D;
    vector<double> grey;
    vector<double> grey_std;
    vector<inf> inf_v;
    feature_calculate(inf_v,y_n,overlap_level,ratio_v,count_v,D,grey,grey_std,connected_region_final);





    Feature feature_curr;
    Chart chart_curr;
    Each_line E_curr;
    feature_curr.y_n = y_n;
    feature_curr.overlap_level = overlap_level;
    feature_curr.ratio_v = ratio_v;
    feature_curr.count_v = count_v;
    feature_curr.D = D;
    feature_curr.grey_mean = grey;
    feature_curr.grey_std = grey_std;
   // Cov_calculate(chart_curr,feature_curr);
    //E_curr = E_calculate(feature_curr);
    bool method=false;
    vector<bool> classify;

    classify = classify_glio(chart,E,chart2,E2,chart_curr,E_curr,feature_curr,method);
    for(int i=0;i<classify.size();i++)
    {
        cout<<"classify = "<<classify[i]<<endl;
    }

    if(im_cropped){delete []im_cropped;im_cropped=0;}

}
void find_relation(QHash<vector<Coordinate>,vector<Coordinate> > &relation,vector<Coordinate> &curr,vector<Coordinate> &tmp,vector<V3DLONG> &reminder2,QHash<Coordinate,int> &map_index)
{
    if(relation[curr].size()==0)return;
    for(int y = 0;y<relation[curr].size();y++)
    {
        tmp.push_back(relation[curr][y]);
    }
    int g = map_index[relation[curr][0]];
    reminder2[g-1] = 1;
    find_relation(relation,relation[curr],tmp,reminder2,map_index);
    return;

}
bool if_is_connect(Coordinate &curr,Coordinate &b,vector<vector<vector<V3DLONG> > > &mark3D)
{

    double dist = NTDIS(curr,b);
    if(dist<3)
    {
        return true;//this
        if(mark3D[curr.z][curr.y][curr.x]==1)
        {
            mark3D[curr.z][curr.y][curr.x]==0;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
bool feature_calculate(vector<inf> &inf_v,vector<double> &y_n,vector<double> &overlap_level,vector<double> &ratio_v,vector<double> &count_v,vector<double> &D,vector<double> &grey,vector<double> &grey_std,vector<vector<Coordinate> >&connected_region_final)
{
    int n=2;
   // cout<<"  <1>.the most big two floor is next to each other or not "<<endl;
    int ind1;
    int ind2;
    vector<Max_level> two_level;
    for(int i=0;i<connected_region_final.size();i++)
    {
        QHash<int,int> hashlinker;
        for(int j=0;j<connected_region_final[i].size();j++)
        {
            Coordinate curr = connected_region_final[i][j];
            hashlinker[curr.z]++;
        }
        int max1=0;
        int max2=0;
        ind1=0;
        ind2=0;

        for(QHash<int,int>::iterator it=hashlinker.begin();it!=hashlinker.end();it++)
        {
            if(it.value()>max1)
            {
                max1 = it.value();
                ind1 = it.key();
            }
        }
        for(QHash<int,int>::iterator it=hashlinker.begin();it!=hashlinker.end();it++)
        {
            if(it.value()==max1)continue;
            if(it.value()>max2)
            {
                max2 = it.value();
                ind2 = it.key();
            }
        }
        Max_level max_l;
        //cout<<"max = "<<max1<<"  "<<max2<<endl;
        //cout<<"ind = "<<ind1<<"  "<<ind2<<endl;
        max_l.level1=ind1;
        max_l.level2=ind2;
        two_level.push_back(max_l);
        if((ind1-ind2==1)||(ind2-ind1==1))
        {
            y_n.push_back(true);
        }
        else
        {
            //v3d_msg("check");
            y_n.push_back(false);
        }
    }


  //  cout<<"  <2><3><4>.calculate overlap,ratio and volume of this two level "<<endl;
    double maxx;
    double maxy;
    double minx;
    double miny;
    double count_level,count_tmp;
    for(int i=0;i<connected_region_final.size();i++)
    {
        count_level=0;
        count_tmp=0;
        maxx=0;
        maxy=0;
        minx=100000000;
        miny=100000000;
        for(int j=0;j<connected_region_final[i].size();j++)
        {
            Coordinate curr = connected_region_final[i][j];
            //cout<<"curr.z = "<<curr.z<<endl;
            //cout<<"two_level[i] = "<<two_level[i].level1<<"  "<<two_level[i].level2<<endl;
            count_level++;
            if(((curr.z!=two_level[i].level1)&&(curr.z!=two_level[i].level2))||two_level[i].level2==0||two_level[i].level1==0)
            {
                continue;
            }
            count_tmp++;
            if(curr.x>maxx)
            {
                maxx=curr.x;
            }
            if(curr.y>maxy)
            {
                maxy=curr.y;
            }
        }
        count_v.push_back(count_level);
        for(int j=0;j<connected_region_final[i].size();j++)
        {
            Coordinate curr = connected_region_final[i][j];
            //cout<<"curr.z = "<<curr.z<<endl;
            //cout<<"two_level[i] = "<<two_level[i].level1<<"  "<<two_level[i].level2<<endl;
            if(((curr.z!=two_level[i].level1)&&(curr.z!=two_level[i].level2))||two_level[i].level2==0||two_level[i].level1==0)
            {
                //ratio_v.push_back(-1);
                //overlap_level.push_back(-1);
                continue;
            }
            if(curr.x<minx)
            {
                minx=curr.x;
            }
            if(curr.y<miny)
            {
                miny=curr.y;
            }
        }
        double minus_x=maxx-minx+1;
        double minus_y=maxy-miny+1;
        double overl=count_tmp/(minus_x*minus_y*2);
        cout<<"x/y = "<<minus_x<<"  "<<minus_y<<endl;

        double ratio1=minus_x/minus_y;
        double ratio2=minus_y/minus_x;
        double ratio;
//        if(ratio1>ratio2)
//            ratio=ratio1;
//        else
//            ratio=ratio2;
        ratio = ratio1;
        ratio_v.push_back(ratio);
        overlap_level.push_back(overl);

    }


    cout<<"size = "<<overlap_level.size()<<endl;
    cout<<"connected_region_final.size() = "<<connected_region_final.size()<<endl;









    QList<ImageMarker> marker_all;
    QList<QList<ImageMarker> > marker_all_each;
    QList<ImageMarker> center;
    int size_thresh = 80;

    for(V3DLONG l=0;l<connected_region_final.size();l++)
    {
        QList<ImageMarker> marker;
        for(int f=0;f<connected_region_final[l].size();f++)
        {

            ImageMarker m;
            m.x = connected_region_final[l][f].x;
            m.y = connected_region_final[l][f].y;
            m.z = connected_region_final[l][f].z;
            m.radius = 0.5;
            m.type = n;
            marker.push_back(m);
            marker_all.push_back(m);
        }
        marker_all_each.push_back(marker);
        //writeMarker_file(QString("marker"+QString::number(l)+".marker"),marker);
    }
    //writeMarker_file(QString("marker_all.marker"),marker_all);
    vector<MyMarker*> inswc;
    vector<Soma> soma_v;


   // cout<<"  <5>.calculate D "<<endl;


  //  cout<<"find_center"<<endl;
    for(V3DLONG l=0;l<connected_region_final.size();l++)
    {
        double sumx=0;
        double sumy=0;
        double sumz=0;
        Soma soma;
        soma.x_b = 10000000;
        soma.y_b = 10000000;
        soma.z_b = 10000000;
        soma.x_e = 0;
        soma.y_e = 0;
        soma.z_e = 0;
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            sumx = sumx + connected_region_final[l][f].x;
            sumy = sumy + connected_region_final[l][f].y;
            sumz = sumz + connected_region_final[l][f].z;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].x<soma.x_b)soma.x_b = connected_region_final[l][f].x;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].y<soma.y_b)soma.y_b = connected_region_final[l][f].y;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].z<soma.z_b)soma.z_b = connected_region_final[l][f].z;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].x>soma.x_e)soma.x_e = connected_region_final[l][f].x;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].y>soma.y_e)soma.y_e = connected_region_final[l][f].y;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].z>soma.z_e)soma.z_e = connected_region_final[l][f].z;
        }
        ImageMarker m;
        MyMarker* m2 = new MyMarker;
        m.x = sumx/connected_region_final[l].size();
        m.y = sumy/connected_region_final[l].size();
        m.z = sumz/connected_region_final[l].size();
        m.radius = 0.5;
        m.type = n;
        m2->x = m.x;
        m2->y = m.y;
        m2->z = m.z;
        m2->type = n;
        inswc.push_back(m2);
        center.push_back(m);
        soma_v.push_back(soma);
    }
    vector<double> max_dis_v;
    for(int d=0;d<connected_region_final.size();d++)
    {
        double sum_dis=0;
        double sum_dis2=0;
        double mean_dis=0;
        double max_dis = 0;
        for(int f=0;f<connected_region_final[d].size();f++)
        {
            double dis = NTDIS(center[d],connected_region_final[d][f]);
            if(dis>max_dis)
            {
                max_dis = dis;
            }
            sum_dis=sum_dis+dis;
        }
        mean_dis = sum_dis/connected_region_final[d].size();

        for(int f=0;f<connected_region_final[d].size();f++)
        {

            double dis = NTDIS(center[d],connected_region_final[d][f]);

            sum_dis2 = sum_dis2+(dis-mean_dis)*(dis-mean_dis);
        }
        max_dis_v.push_back(max_dis);
        double Dd = sum_dis2/(connected_region_final[d].size()-1);
        D.push_back(Dd);

    }


    QList<ImageMarker> center_choose;
    for(V3DLONG d=0;d<connected_region_final.size();d++)
    {
        double x_dis = soma_v[d].x_e - soma_v[d].x_b;
        double y_dis = soma_v[d].y_e - soma_v[d].y_b;
     //   cout<<"d = "<<d<<endl;
 //       cout<<"connected_region_final[d].size() = "<<connected_region_final[d].size()<<endl;
 //       cout<<"max_dis = "<<max_dis_v[d]<<endl;
   //     cout<<"D = "<<D[d]<<endl;
        if(connected_region_final[d].size()<size_thresh)
            continue;
        //if(marker_all_each[d].size()/(max_dis_v[d]*max_dis_v[d]*max_dis_v[d]*8)<0.7)
          //  continue;
        if(y_dis-x_dis>7||x_dis-y_dis>7)
            continue;
        if(max_dis_v[d]>20)
            continue;
        if(D[d]>10)
            continue;

        center_choose.push_back(center[d]);


    }

 //   cout<<"  <6>.calculate mean grey "<<endl;

    double mean_grey;
    for(int i=0;i<connected_region_final.size();i++)
    {
        double sum_grey=0;
        for(int d=0;d<connected_region_final[i].size();d++)
        {
            sum_grey = sum_grey + connected_region_final[i][d].bri;
        }
        mean_grey = sum_grey/connected_region_final[i].size();
        grey.push_back(mean_grey);
    }
    cout<<"  <7>.calculate std grey "<<endl;
    double std_grey;
    for(int i=0;i<connected_region_final.size();i++)
    {
        double sum_grey=0;
        for(int d=0;d<connected_region_final[i].size();d++)
        {
            sum_grey = (connected_region_final[i][d].bri-grey[i])*(connected_region_final[i][d].bri-grey[i]) + sum_grey;
        }
        std_grey = sum_grey/(connected_region_final[i].size()-1);
      //  cout<<"sum_grey = "<<sum_grey<<"      "<<(connected_region_final[i].size()-1)<<"    "<<i<<endl;

        grey_std.push_back(sqrt(std_grey));
    }
       // v3d_msg("gggggggggggg");
//    for(int i=0;i<D.size();i++)
//    {
//        cout<<"y_n = "<<y_n[i]<<endl;
//        cout<<"D = "<<D[i]<<endl;
//        cout<<"overlap_level = "<<overlap_level[i]<<endl;
//        cout<<"ratio_v = "<<ratio_v[i]<<endl;
//        cout<<"count_v = "<<count_v[i]<<endl;
//        cout<<"grey_mean = "<<grey[i]<<endl;
//        cout<<"grey_std = "<<grey_std[i]<<endl;
//        cout<<"*************************************************"<<endl;
//    }
 //   cout<<"  <7>.get information from folder "<<endl;
    for(int i=0;i<connected_region_final.size();i++)
    {
        //vector<inf> inf_tmp_v;
       // for(int d=0;d<connected_region_final[i].size();d++)
        //{
            inf inf_tmp;
            inf_tmp.inf1 =  connected_region_final[i][0].inf1;
            inf_tmp.name = connected_region_final[i][0].name;
            inf_v.push_back(inf_tmp);
      //  }
        //inf_v.push_back(inf_tmp_v);
    }



}
bool export_training_data(const QString &fileOpenName,Chart &chart,Each_line &E)
{


    QFile qf(fileOpenName);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(fileOpenName));
#endif
        return false;
    }

    V3DLONG k=0,j=1;
    while (! qf.atEnd())
    {
        char curline[2000];

        qf.readLine(curline, sizeof(curline));
        k++;
        {

            QStringList qsl = QString(curline).trimmed().split("   ");
            int qsl_count=qsl.size();
            cout<<"qls.size = "<<qsl_count<<endl;
            if (qsl_count<6)   continue;

            switch(j)
            {
                case 1: E.x1 = qsl[0].toFloat();
                        chart.first_line.x1 = qsl[1].toFloat();
                        chart.first_line.x2 = qsl[2].toFloat();
                        chart.first_line.x3 = qsl[3].toFloat();
                        chart.first_line.x4 = qsl[4].toFloat();
                        chart.first_line.x5 = qsl[5].toFloat();break;
                case 2: E.x2 = qsl[0].toFloat();
                        chart.second_line.x1 = qsl[1].toFloat();
                        chart.second_line.x2 = qsl[2].toFloat();
                        chart.second_line.x3 = qsl[3].toFloat();
                        chart.second_line.x4 = qsl[4].toFloat();
                        chart.second_line.x5 = qsl[5].toFloat();break;
                case 3: E.x3 = qsl[0].toFloat();
                        chart.third_line.x1 = qsl[1].toFloat();
                        chart.third_line.x2 = qsl[2].toFloat();
                        chart.third_line.x3 = qsl[3].toFloat();
                        chart.third_line.x4 = qsl[4].toFloat();
                        chart.third_line.x5 = qsl[5].toFloat();break;
                case 4: E.x4 = qsl[0].toFloat();
                        chart.forth_line.x1 = qsl[1].toFloat();
                        chart.forth_line.x2 = qsl[2].toFloat();
                        chart.forth_line.x3 = qsl[3].toFloat();
                        chart.forth_line.x4 = qsl[4].toFloat();
                        chart.forth_line.x5 = qsl[5].toFloat();break;
                case 5: E.x5 = qsl[0].toFloat();
                        chart.fifth_line.x1 = qsl[1].toFloat();
                        chart.fifth_line.x2 = qsl[2].toFloat();
                        chart.fifth_line.x3 = qsl[3].toFloat();
                        chart.fifth_line.x4 = qsl[4].toFloat();
                        chart.fifth_line.x5 = qsl[5].toFloat();break;
            default:break;

            }
            j++;

        }
    }
    cout << E.x1 <<"    "<<chart.first_line.x1<<"    "<<chart.first_line.x2<<"    "<<chart.first_line.x3<<"    "<<chart.first_line.x4<<"    "<<chart.first_line.x5<<endl;
    cout << E.x2 <<"    "<<chart.second_line.x1<<"    "<<chart.second_line.x2<<"    "<<chart.second_line.x3<<"    "<<chart.second_line.x4<<"    "<<chart.second_line.x5<<endl;
    cout << E.x3 <<"    "<<chart.third_line.x1<<"    "<<chart.third_line.x2<<"    "<<chart.third_line.x3<<"    "<<chart.third_line.x4<<"    "<<chart.third_line.x5<<endl;
    cout << E.x4 <<"    "<<chart.forth_line.x1<<"    "<<chart.forth_line.x2<<"    "<<chart.forth_line.x3<<"    "<<chart.forth_line.x4<<"    "<<chart.forth_line.x5<<endl;
    cout << E.x5 <<"    "<<chart.fifth_line.x1<<"    "<<chart.fifth_line.x2<<"    "<<chart.fifth_line.x3<<"    "<<chart.fifth_line.x4<<"    "<<chart.fifth_line.x5<<endl;
    return true;
}
vector<bool> classify_glio(Chart &chart1,Each_line &E1,Chart &chart2,Each_line &E2,Chart &chart_curr,Each_line &E_curr,Feature &feature_curr,bool &method)
{
    cout<<"oooooooooooooooooooooooo"<<endl;
    vector<bool> classify;
    if(method)
    {
        double dis11 = sqrt( (E1.x1 - E_curr.x1)*(E1.x1 - E_curr.x1) + (chart1.first_line.x1-chart_curr.first_line.x1)*(chart1.first_line.x1-chart_curr.first_line.x1) +
                (chart1.first_line.x2-chart_curr.first_line.x2)*(chart1.first_line.x2-chart_curr.first_line.x2) + (chart1.first_line.x3-chart_curr.first_line.x3 )*(chart1.first_line.x3-chart_curr.first_line.x3)+
                (chart1.first_line.x4-chart_curr.first_line.x4)*(chart1.first_line.x4-chart_curr.first_line.x4) + (chart1.first_line.x5-chart_curr.first_line.x5)*(chart1.first_line.x5-chart_curr.first_line.x5) );

        double dis12 = sqrt( (E1.x2 - E_curr.x2)*(E1.x2 - E_curr.x2) + (chart1.second_line.x1-chart_curr.second_line.x1)*(chart1.second_line.x1-chart_curr.second_line.x1) +
                (chart1.second_line.x2-chart_curr.second_line.x2)*(chart1.second_line.x2-chart_curr.second_line.x2) + (chart1.second_line.x3-chart_curr.second_line.x3 )*(chart1.second_line.x3-chart_curr.second_line.x3)+
                (chart1.second_line.x4-chart_curr.second_line.x4)*(chart1.second_line.x4-chart_curr.second_line.x4) + (chart1.second_line.x5-chart_curr.second_line.x5)*(chart1.second_line.x5-chart_curr.second_line.x5) );

        double dis13 = sqrt( (E1.x3 - E_curr.x3)*(E1.x3 - E_curr.x3) + (chart1.third_line.x1-chart_curr.third_line.x1)*(chart1.third_line.x1-chart_curr.third_line.x1) +
                (chart1.third_line.x2-chart_curr.third_line.x2)*(chart1.third_line.x2-chart_curr.third_line.x2) + (chart1.third_line.x3-chart_curr.third_line.x3 )*(chart1.third_line.x3-chart_curr.third_line.x3)+
                (chart1.third_line.x4-chart_curr.third_line.x4)*(chart1.third_line.x4-chart_curr.third_line.x4) + (chart1.third_line.x5-chart_curr.third_line.x5)*(chart1.third_line.x5-chart_curr.third_line.x5) );

        double dis14 = sqrt( (E1.x4 - E_curr.x4)*(E1.x4 - E_curr.x4) + (chart1.forth_line.x1-chart_curr.forth_line.x1)*(chart1.forth_line.x1-chart_curr.forth_line.x1) +
                (chart1.forth_line.x2-chart_curr.forth_line.x2)*(chart1.forth_line.x2-chart_curr.forth_line.x2) + (chart1.forth_line.x3-chart_curr.forth_line.x3 )*(chart1.forth_line.x3-chart_curr.forth_line.x3)+
                (chart1.forth_line.x4-chart_curr.forth_line.x4)*(chart1.forth_line.x4-chart_curr.forth_line.x4) + (chart1.forth_line.x5-chart_curr.forth_line.x5)*(chart1.forth_line.x5-chart_curr.forth_line.x5) );

        double dis15 = sqrt( (E1.x5 - E_curr.x5)*(E1.x5 - E_curr.x5) + (chart1.fifth_line.x1-chart_curr.fifth_line.x1)*(chart1.fifth_line.x1-chart_curr.fifth_line.x1) +
                (chart1.fifth_line.x2-chart_curr.fifth_line.x2)*(chart1.fifth_line.x2-chart_curr.fifth_line.x2) + (chart1.fifth_line.x3-chart_curr.fifth_line.x3 )*(chart1.fifth_line.x3-chart_curr.fifth_line.x3)+
                (chart1.fifth_line.x4-chart_curr.fifth_line.x4)*(chart1.fifth_line.x4-chart_curr.fifth_line.x4) + (chart1.fifth_line.x5-chart_curr.fifth_line.x5)*(chart1.fifth_line.x5-chart_curr.fifth_line.x5) );


        double dis21 = sqrt( (E2.x1 - E_curr.x1)*(E2.x1 - E_curr.x1) + (chart2.first_line.x1-chart_curr.first_line.x1)*(chart2.first_line.x1-chart_curr.first_line.x1) +
                (chart2.first_line.x2-chart_curr.first_line.x2)*(chart2.first_line.x2-chart_curr.first_line.x2) + (chart2.first_line.x3-chart_curr.first_line.x3 )*(chart2.first_line.x3-chart_curr.first_line.x3)+
                (chart2.first_line.x4-chart_curr.first_line.x4)*(chart2.first_line.x4-chart_curr.first_line.x4) + (chart2.first_line.x5-chart_curr.first_line.x5)*(chart2.first_line.x5-chart_curr.first_line.x5) );

        double dis22 = sqrt( (E2.x2 - E_curr.x2)*(E2.x2 - E_curr.x2) + (chart2.second_line.x1-chart_curr.second_line.x1)*(chart2.second_line.x1-chart_curr.second_line.x1) +
                (chart2.second_line.x2-chart_curr.second_line.x2)*(chart2.second_line.x2-chart_curr.second_line.x2) + (chart2.second_line.x3-chart_curr.second_line.x3 )*(chart2.second_line.x3-chart_curr.second_line.x3)+
                (chart2.second_line.x4-chart_curr.second_line.x4)*(chart2.second_line.x4-chart_curr.second_line.x4) + (chart2.second_line.x5-chart_curr.second_line.x5)*(chart2.second_line.x5-chart_curr.second_line.x5) );

        double dis23 = sqrt( (E2.x3 - E_curr.x3)*(E2.x3 - E_curr.x3) + (chart2.third_line.x1-chart_curr.third_line.x1)*(chart2.third_line.x1-chart_curr.third_line.x1) +
                (chart2.third_line.x2-chart_curr.third_line.x2)*(chart2.third_line.x2-chart_curr.third_line.x2) + (chart2.third_line.x3-chart_curr.third_line.x3 )*(chart2.third_line.x3-chart_curr.third_line.x3)+
                (chart2.third_line.x4-chart_curr.third_line.x4)*(chart2.third_line.x4-chart_curr.third_line.x4) + (chart2.third_line.x5-chart_curr.third_line.x5)*(chart2.third_line.x5-chart_curr.third_line.x5) );

        double dis24 = sqrt( (E2.x4 - E_curr.x4)*(E2.x4 - E_curr.x4) + (chart2.forth_line.x1-chart_curr.forth_line.x1)*(chart2.forth_line.x1-chart_curr.forth_line.x1) +
                (chart2.forth_line.x2-chart_curr.forth_line.x2)*(chart2.forth_line.x2-chart_curr.forth_line.x2) + (chart2.forth_line.x3-chart_curr.forth_line.x3 )*(chart2.forth_line.x3-chart_curr.forth_line.x3)+
                (chart2.forth_line.x4-chart_curr.forth_line.x4)*(chart2.forth_line.x4-chart_curr.forth_line.x4) + (chart2.forth_line.x5-chart_curr.forth_line.x5)*(chart2.forth_line.x5-chart_curr.forth_line.x5) );

        double dis25 = sqrt( (E2.x5 - E_curr.x5)*(E2.x5 - E_curr.x5) + (chart2.fifth_line.x1-chart_curr.fifth_line.x1)*(chart2.fifth_line.x1-chart_curr.fifth_line.x1) +
                (chart2.fifth_line.x2-chart_curr.fifth_line.x2)*(chart2.fifth_line.x2-chart_curr.fifth_line.x2) + (chart2.fifth_line.x3-chart_curr.fifth_line.x3 )*(chart2.fifth_line.x3-chart_curr.fifth_line.x3)+
                (chart2.fifth_line.x4-chart_curr.fifth_line.x4)*(chart2.fifth_line.x4-chart_curr.fifth_line.x4) + (chart2.fifth_line.x5-chart_curr.fifth_line.x5)*(chart2.fifth_line.x5-chart_curr.fifth_line.x5) );
        double dis1 = dis11+dis12+dis13+dis14+dis15;
        double dis2 = dis21+dis22+dis23+dis24+dis25;

//        if(dis1>dis2){return true;}
//        else{return false;}

    }
    else
    {
        int size = feature_curr.count_v.size();
        for(int i=0;i<size;i++)
        {
            double dis1 = sqrt( (E1.x1 - feature_curr.y_n[i])*(E1.x1 - feature_curr.y_n[i]) + (E1.x2 - feature_curr.ratio_v[i])*(E1.x2 - feature_curr.ratio_v[i]) + (E1.x3 - feature_curr.overlap_level[i])*(E1.x3 - feature_curr.overlap_level[i])
                                + (E1.x4 - feature_curr.grey_std[i])*(E1.x4 - feature_curr.grey_std[i]) + (E1.x5 - feature_curr.count_v[i])*(E1.x5 - feature_curr.count_v[i]) );

            double dis2 = sqrt( (E2.x1 - feature_curr.y_n[i])*(E2.x1 - feature_curr.y_n[i]) + (E2.x2 - feature_curr.ratio_v[i])*(E2.x2 - feature_curr.ratio_v[i]) + (E2.x3 - feature_curr.overlap_level[i])*(E2.x3 - feature_curr.overlap_level[i])
                                + (E2.x4 - feature_curr.grey_std[i])*(E2.x4 - feature_curr.grey_std[i]) + (E2.x5 - feature_curr.count_v[i])*(E2.x5 - feature_curr.count_v[i]) );



            if(dis1>dis2)
            {
                classify.push_back(1);
            }
            else
            {
                classify.push_back(0);
            }


        }
        return classify;

    }

}
