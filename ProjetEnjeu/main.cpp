#include "PrecHeaders.h"
#include "CImgProvider.h"
#include "features2d.hpp"
#include "nonfree.hpp"

using namespace std;




vector<cv::DMatch> TriBulle(vector<cv::DMatch> tableau, int longueur);
vector <cv::DMatch> echanger(vector<cv::DMatch> tableau, int i, int j);
int decoupe(vector<cv::DMatch> tableau, int debut, int fin);
vector<cv::DMatch> TriRapide(vector<cv::DMatch> tableau, int longueur);
vector<cv::DMatch> TriRapideParties(vector<cv::DMatch> tableau, int debut, int fin);


int main(int argc, const char **argv)
{  



/*##################### Paramètres ############################################*/

	/*##### Généraux ###############################*/

//	int KSize = 11;   // Taille du noyau pour le filtre gaussien / -> Impair !
	int tri = 3; // Les 3 tris implémentés = 1 : critère 3*dist_min; 2 : Tri à bulle; 3 : Tri rapide
	int pourcentage = 10;   //pourcentage de paires retenues


	/*####### SIFT : détéction et extraction ######*/

	int nfeatures = 50;
	int nOctaveLayers = 3;
	double contrastThreshold = 0.15;
	double edgeThreshold = 5;
	double sigma = 1.6;
	


/*##################### Variables #############################################*/
	cv::Mat out, currImg, refImg, stabImg, tmpImg, imgMatches, match,
			currDescripteur, refDescripteur,
			T;

	vector<cv::KeyPoint> refKeyPoints, currKeyPoints;
	vector<cv::Point2f>	 refPoint;
	vector<cv::Point2f>	 currPoint;
	vector<cv::DMatch>	 matches, matches_tri, good_matches;


/*#################### Instances    #######################*/

	cv::SiftFeatureDetector		detecteur(nfeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);		//	cv::SiftFeatureDetector::SiftFeatureDetector 	(
													//	int nfeatures = 0
													//	int nOctaveLayers = 3
													//	double contrastThreshold = 0.04
													//	double edgeThreshold = 10
													//	double sigma = 1.6 )


	
	cv::SiftDescriptorExtractor	extracteur(nfeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);
	cv::FlannBasedMatcher		matcher;
	


/*#################### Gestion des entrées sorties vidéos ####################*/

	// La vidéo en entrée
    const string input = "../data/image_.avi";
    CImgProvider provider(input);
	cv::VideoCapture videoinput(input); // Image provider helps you extract images from video file

	// Les vidéos en sortie
	cv::VideoWriter writer("../data/output.avi",CV_FOURCC('x','v','i','d'),videoinput.get(CV_CAP_PROP_FPS),cvSize(provider.getW(),provider.getH()));

	cv::VideoWriter writer_match("../data/match.avi",CV_FOURCC('x','v','i','d'),videoinput.get(CV_CAP_PROP_FPS),cvSize(2*provider.getW(),provider.getH()));

	//cout << provider.getCodec() << endl;    // Just for fun check codec version

	/*########## Masque pour les éléments sur-imprimés ########*/
	

	
	
	/*####### Traitement de la 1ère image (Référence)  ###########*/

	cv::Mat mask;

	refKeyPoints.clear();

	while(refKeyPoints.size() < 10)
	{
		provider.getNext(tmpImg);

	tmpImg.copyTo(refImg);
	//cv::GaussianBlur(out, refImg, cv::Size(KSize,KSize),0,0);

	mask = cv::Mat::ones(refImg.rows, refImg.cols, CV_8UC1);
	
//	cout << "test" << endl;
	for (int i = 0; i < 320; i++)
	{
		for(int j = 450; j < 480; j++)  // Maximum : (0, 632, 0, 480)  ->632 ??   Bien : 0-320,  450-480
		{
		//	mask.at<uchar>(j, i) = 0;
		}
	}


	//cout << "test" << endl;
	detecteur.detect(refImg, refKeyPoints, mask);

	extracteur.compute(refImg, refKeyPoints, refDescripteur);



	//cout << "ttest" << endl;
//	cout << "Nombre de KeyPoints : " << refKeyPoints.size() << endl;

	imshow("difference", refImg);
	//writer << out;



		
	}

		cv::drawKeypoints(refImg, refKeyPoints, out, CV_RGB(255,0,0));
		writer << refImg;
	
/*#################### Parcours de la vidéo image par image ####################*/

    while (provider.getNext(tmpImg))        // GetNext returns false if no more images can be retrieved from video file
    {
		tmpImg.copyTo(currImg);
		//cv::GaussianBlur(out, currImg, cv::Size(KSize,KSize),0,0);
		
		detecteur.detect(currImg, currKeyPoints, mask);
		
		extracteur.compute(currImg, currKeyPoints, currDescripteur);
	
		matcher.match( refDescripteur, currDescripteur, matches );
		
		cv::drawKeypoints(currImg, currKeyPoints, out, CV_RGB(255,0,0));

		//cout << "Nombre de KeyPoints : " << currKeyPoints.size() << endl;


	refPoint.clear();
	currPoint.clear();

	/*########## Suppression des paires erronnées ######*/
	if(tri == 1)
	{
		double max_dist = 0; double min_dist = 10000;
	
	  // Calcul des distances min et max entre 2 paires
		for( int i = 0; i < refDescripteur.rows; i++ )
		{ 
			double dist = matches[i].distance;
		    if( dist < min_dist ) min_dist = dist;
		    if( dist > max_dist ) max_dist = dist;
		}
  
		good_matches.clear();

		for(int i = 0; i < refDescripteur.rows; i++)
		{
			if( matches[i].distance < 3*min_dist )
    
			{
				good_matches.push_back( matches[i]);
			}
		}  

	}

	else if(tri == 2)
	{
		matches_tri = TriBulle(matches, matches.size());
	}

	else if(tri == 3)
	{
		//cout << matches.size() << endl;
		matches_tri = TriRapide(matches, matches.size());
	}


	
	/*######  On retient seulement les meilleurs #########*/

	good_matches.clear();
	for (int i = 0; i < (int)(matches_tri.size()*pourcentage/100); i++)
		good_matches.push_back(matches_tri[i]);

		//cout << "Nombre de paires : " << matches_tri.size() << endl;
		//cout << "Nombre de paires retenues : " << good_matches.size() << endl;
	for(int i = 0 ; i < good_matches.size() ; i++)
	{
		//cout << good_matches[i].distance << endl;
	}


	cv::drawMatches( refImg, refKeyPoints, currImg, currKeyPoints,
                        good_matches, imgMatches, cv::Scalar::all(-1), cv::Scalar::all(-1),
                        vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );


	
	for( int i = 0; i < good_matches.size(); i++ )
  {
    //-- Get the keypoints from the good matches
    refPoint.push_back( refKeyPoints[ good_matches[i].queryIdx ].pt );
    currPoint.push_back( currKeyPoints[ good_matches[i].trainIdx ].pt );
  }
	//cv::drawMatches(refImg, refKeyPoints, out, currKeyPoints, good_matches, match, cv::Scalar::all(-1), CV_RGB(255,0,0));
	cout << "Distance : " << (currPoint[0].x - refPoint[0].x)*(currPoint[0].x - refPoint[0].x) + (currPoint[0].y - refPoint[0].y)*(currPoint[0].y - refPoint[0].y) << endl;

	/*###### Calcul de la transformation et Stabilisation  #########*/

	T = cv::estimateRigidTransform(refPoint,currPoint, false);		// true = Modèle Affine, 6 inconnues
																	// false = Modèle simplifié, 5 inconnues


	cout << T << endl;

	T.at<double>(0,0) = 1; 
	T.at<double>(0,1) =0;
	T.at<double>(1,1) =1;
	T.at<double>(1,0) =0;

	currImg.copyTo(stabImg);
	cv::warpAffine(currImg, stabImg, T, cvSize(currImg.cols, currImg.rows), cv::WARP_INVERSE_MAP);

	
/*#################### Affichage ####################*/
 


        imshow("difference", imgMatches);    // simple display call
        cv::waitKey(10);              // Wait for key press for n ms (0 means forever)

        // And save it to video file
        writer << stabImg;
		writer_match << imgMatches;
    }

    return 0;
	}









vector<cv::DMatch> TriBulle(vector<cv::DMatch> tableau, int longueur)
{

    int i;
    bool modif;

    do
    {
        modif = false;
        for(i=0; i<longueur-1; i++)
        {
            if (tableau[i].distance > tableau[i+1].distance)
            {
                tableau = echanger(tableau, i, i+1);
                modif = true;
            }
        }
        longueur--;
    }
    while(modif);

    return tableau;
}

vector<cv::DMatch> echanger(vector<cv::DMatch> tableau, int i, int j)
{
    cv::DMatch temp;
    temp = tableau[i];
    tableau[i] = tableau[j];
    tableau[j] = temp;
	return tableau;
} 

int decoupe(vector<cv::DMatch> tableau, int debut, int fin)
{
	int position = debut;
	cv::DMatch pivot = tableau[debut];
	int i;

	for (i=debut+1; i<=fin; i++)
	{
		if(tableau[i].distance<pivot.distance)
		{
			
			tableau = echanger(tableau, position, i);
			position++;
		}
	
	
	}
	tableau = echanger(tableau, position, debut);
	return position;                     //la position du pivot est retournée
}

vector<cv::DMatch> TriRapideParties(vector<cv::DMatch> tableau, int debut, int fin)
{
	vector<cv::DMatch> temp1, temp2;
	if(debut<fin)
	{


		int position = debut+1;
	cv::DMatch pivot = tableau[debut];
	int i;

	for (i=debut+1; i<=fin; i++)
	{
		if(tableau[i].distance<pivot.distance)
		{
			
			tableau = echanger(tableau, position, i);
			position++;
		}
	
	
	}
	position--;
	tableau = echanger(tableau, position, debut);
	
		
		temp1 = TriRapideParties(tableau, debut, position-1);
		temp2 = TriRapideParties(tableau, position+1, fin);
		temp1.push_back(tableau[position]);
		temp1.insert(temp1.end(), temp2.begin(), temp2.end());
		return temp1;
	}
	
	else if (debut == fin)
	{
		temp1.push_back(tableau[debut]);
		return temp1;
	}

	else 
	{

	temp1.clear();
	return temp1;
	}
	
}


vector<cv::DMatch> TriRapide(vector<cv::DMatch> tableau, int longueur)
{
	return TriRapideParties(tableau, 0, longueur-1);
}

