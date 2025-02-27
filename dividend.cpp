//
// Created by Cephas Svosve on 17/6/2021.
//

#include "dividend.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <iostream>
#include <boost/random.hpp>



//get dividend growth rate i.e average of the percentage change in dividend paid-out, dD(t)/D(t-1).

double Div::getGrowthRate(int tickerID)
{
 return growthRate[tickerID];
}





//get volatility i.e stdev of the percentage changes in dividend paid-out, dD(t)/D(t-1).

double Div::getVolatility(int tickerID)
{
    return volatility[tickerID];
}




//we retrieve the autocorrelation of a particular asset by this function

double Div::getAutoCorr(int tickerID)
{
    return autoCorr[tickerID];
}





//this is a setter for the average growth rate of an asset's growth rate

void Div::setGrowthRate(double x)
{
 growthRate.push_back(x);
}





//this is a setter for saving the volatility of an asset's dividend process

void Div::setVolatility(double x)
{
 volatility.push_back(x);
}





//this is a setter for saving the auto-correlation of an asset's dividend process

void Div::setAutoCorr(double x)
{
 autoCorr.push_back(x);
}





//work-in-progress-*the function is meant to retrieve the cross-sectional correlation from the market object 

void Div::setCrossCorr()
{
 // std::cout <<  "Div " << crossCorrMat << std::endl;
}





//because our system depends on accuracy of the randomness of the random matrix,
//we create a function that carefully generates a purely random matrix,
//i.e. auto-correlation is 0 precise to the order of e-03 as well as the cross-sectional correlation



MatrixXd Div::generateWhiteNoise(int numOfAssets, int numOfTicks)
{

    MatrixXd randoms(numOfAssets,numOfTicks);
    VectorXd a;



    //algorithm for generating random numbers that are shuffled continuously through seeding them on time

    time_t now = time(0);
    boost::random::mt19937 gen{static_cast<uint32_t>(now)};
    boost::normal_distribution<> nd(0.0, clock.getDt());
    boost::variate_generator<boost::mt19937&,
    boost::normal_distribution<> > var_nor(gen, nd);



    //we generate the matrix of random numbers

    for(int rw=0; rw < numOfAssets; rw++)
    {

     
        //we make sure the naturally occurring auto-correlation is sufficiently small by using a do-while loop
        
     do
        {
            //here we load each row with appropriate vector of random numbers
            a = VectorXd(numOfTicks);
      
      
            for(int i = 0; i < numOfTicks; ++i)
            {
                a(i) = var_nor();
            }


        }
        while (abs(lateralcorrcoef(a)) > 0.001);

        randoms.row(rw) = a;

    }


    //We then remove any cross-sectional correlation

    return verticallyWhiten(randoms);
}





//this function removes cross-sectional correlation by multiplying a matrix
// by the inverse of the lower cholesky decomposition of its correlation matrix


MatrixXd Div::verticallyWhiten(MatrixXd mx)
{
    Matrix crossCorrs = crossCorr(mx);
    LLT<MatrixXd> llt(crossCorrs);
    MatrixXd L = llt.matrixL();
    MatrixXd vWhite = L.inverse() * mx;


    return vWhite;
}





//this function calculates the coefficient of determination -correlation- between two vectors
//it requires the two vectors x and y, then computes the variables required for the formula
//i.e. variables xx = x^2  ,xy = x*y  and yy = y^2



double Div::corrcoef(VectorXd x, VectorXd y)
{
    if(x.size() == y.size()) {

        int n = x.size();
     

        //variable xx represents x^2

        double xx[n];
        double xy[n];
        double yy[n];



        //sumx is the sum of random variables x

        double sumx = x.sum();
        double sumy = y.sum();



        //sumxx is the sum of random variables x^2

        double sumxx;
        double sumxy;
        double sumyy;



        for (int i=0; i < x.size(); i++) 
        {
            xx[i] = pow(x[i],2);
            xy[i] = x[i] * y[i];
            yy[i] = pow(y[i],2);
         
            sumxx = sumxx + xx[i];
            sumxy = sumxy + xy[i];
            sumyy = sumyy + yy[i];
        }


        //this is the formula used to calculate coefficient of determination, *see
        //https://www.jstor.org/stable/2965177?seq=6#metadata_info_tab_contents
        //Reed, William Gardner. “The Coefficient of Correlation.” Publications of the American Statistical Association, 
        //vol. 15, no. 118, 1917, pp. 675–684. JSTOR, www.jstor.org/stable/2965177. Accessed 20 June 2021.

        double r = (n*sumxy - sumx*sumy)/sqrt((n*sumxx-pow(sumx,2))*(n*sumyy-pow(sumy,2)));

        return r;
    }
    else
    {
     cout << "found vectors of different size on computing correlation";
    }

}





//this function calculates the auto-correlation coefficient of a vector of random numbers
//we use it to verify that the naturally occuring auto-correlation in the random numbers-
//-we are using is sufficiently small i.e. of order e-03



double Div::lateralcorrcoef(VectorXd a){


    int n = a.size()-1;


    double x[n];
    double y[n];
    double xx[n];
    double xy[n];
    double yy[n];

    double sumx;
    double sumy;
    double sumxx;
    double sumxy;
    double sumyy;

    for (int i=0; i < n; i++) {

        x[i] = a[i+1];
        y[i] = a[i];
        xx[i] = pow(x[i],2);
        xy[i] = x[i] * y[i];
        yy[i] = pow(y[i],2);


        sumx = sumx + x[i];
        sumy = sumy + y[i];

        sumxx = sumxx + xx[i];
        sumxy = sumxy + xy[i];
        sumyy = sumyy + yy[i];

    }

    double r = (n*sumxy - sumx*sumy)/sqrt((n*sumxx-pow(sumx,2))*(n*sumyy-pow(sumy,2)));
    return r;

}




//this function calculates the correlation matrix from the random matrix 

MatrixXd Div::crossCorr(MatrixXd x){
    int m = x.col(0).size();
    int n = x.row(0).size();

    MatrixXd corrMat(m,m);

    for(int i = 0; i < m; i++){
        for(int j = 0; j <= i; j++){

            corrMat(i,j) = corrcoef( x.row(i),x.row(j));
            corrMat(j,i) = corrMat(i,j);
        }
    }

    return corrMat;
}





//work-in-progress

MatrixXd Div::verticallyColor(MatrixXd x){}

VectorXd Div::laterallyColor(VectorXd x){}

VectorXd Div::standardize(VectorXd x){}





MatrixXd Div::generateColoredNoise(int numOfAssets, int numOfTicks)
{
    MatrixXd X = generateWhiteNoise(numOfAssets, numOfTicks);
    MatrixXd Y = verticallyWhiten(X);


    //vector<double> lC = laterallyColor(Y(0:));
}

