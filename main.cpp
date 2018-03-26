#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
using namespace std;

//Data Structure to Store value of Given Windows
struct window{
	double confidence;
	int x;
	int y;
	int width;
	int bredth;
	int car_number;
};

//Data Structure to be drawn over Image
struct Rect{
	int x;
	int y;
	int width;
	int bredth;
};

//Class to find best results in the given file
class findCar{
	public:

	/*
		Parse the file and save elements in vector of Window
	*/
	void readFile(int fileNumber){
		string currenInputFile("input/");
    	currenInputFile+=to_string(fileNumber);
		_fileLocation = currenInputFile;
		_fileNumber = fileNumber;
		cout << "Current File : " << _fileLocation << endl;
	    bool error=false;
		ifstream fin;
		fin.open(_fileLocation);
		while(true){
			window temp;
			string input;
			getline(fin, input);
			if(!fin) break;
			istringstream buffer(input);
	        buffer >> temp.confidence >> temp.x >> temp.y >> temp.width >> temp.bredth;
	        possibleCars.push_back(temp);
		}
	    if (error) { cout << "file is corrupted..." << endl; }
	}

	/*
		Simple print function for window
	*/
	void printWindow(window &w){
		cout << " 1 : " << w.confidence << "\t" << 
		" | 2 : "<< w.x << "\t" <<
		" | 3 : "<< w.y << "\t" << 
		" | 4 : " << w.width << "\t" << 
		" | 5 : "<< w.bredth << endl;  
	}
	
	/*
		Initilisation Function
	*/
	void init(){
		for(int i=0; i<totalCurrentCases; i++){
			registered.push_back(false);
			possibleCars[i].car_number=0;
		}
		currentMaxCarNumber=1;
		possibleCars[0].car_number=1;
		registered[0]=true;
		auto t = make_pair(possibleCars[0].confidence, 0);
		windowWithMaxConfidence.insert({currentMaxCarNumber, t});
	}

	/*
		Gives the total number of input cases. 
	*/
	void getStats(){
		totalCurrentCases = possibleCars.size();
		// cout << "Total pausible cases : " << totalCurrentCases << endl;
	}

	/*
		Find the ratio of the area overlapping versus total area of the rectangle j
	*/
	float overlapRatio(int i, int j){
		//percent area of j in i based on j
		window rect1 = possibleCars[i];
		window rect2 = possibleCars[j];

		int rect2_x = (rect1.x + rect1.width) - rect2.x;
		int rect2_y = abs((rect1.y - rect1.bredth) - rect2.y);

		float ratio = float(rect2_x*rect2_y)/float(rect2.width*rect2.bredth);
		return ratio;
	}

	/*
		Check the quadrant in which rect j lie inside rect i
	*/
	int checkQuadcrant(int i, int j){
		//check for all the the cases after i
		window case1 = possibleCars[i];
		window case2 = possibleCars[j];
		
		int case1_min_x = case1.x;
		int case1_min_y = case1.y;	
		int case1_max_x = case1.x + case1.width;
		int case1_max_y = case1.y - case1.bredth;
		int case1_mid_x = (case1_min_x + case1_max_x) / 2;
		int case1_mid_y = (case1_min_y + case1_max_y) / 2;
 		
 		//			1   |  2
 		//				|		
 		//				|
 		//        -------------
 		//				|
 		//			4	|  3 
 		//				|

 		//In Quadrant 1 : If this is the case, we say both belong to same car and one with more
 		//				  confidence is saved.	
		if((case1_mid_y < case2.y && case1_min_y > case2.y) && (case1_mid_x > case2.x && case1_min_x < case2.x)){ 
			registered[j] = true;
			possibleCars[j].car_number = possibleCars[i].car_number;
			if(windowWithMaxConfidence[currentMaxCarNumber].first < possibleCars[j].confidence){
					auto t = make_pair(possibleCars[j].confidence, j);
					windowWithMaxConfidence[currentMaxCarNumber] = t;
			}
			return 1;
		}
		//In Quadcrant 2 || 3 || 4	: If this is the case, we check the ratio of area of Rect(j) which is
		//							  lying inside the area of rect(i) to complete area of Rect(j)
		//
		//							  area ( Rect(j) inside Rect(i))
		//					Ratio =	  ------------------------------
		//							  area ( Rect(j) )	 		
		// If this ratio is above 0.7 we consider them to belong to the same car, else different cars
		// The value 0.7 should tuned based on the data used.
		else if((case1_max_y < case2.y && case1_min_y > case2.y) || (case1_max_x > case2.x && case1_min_x < case2.x)){
			//check for the ratio of overlapping area
			//The value has to be tuned according to the data
			if(overlapRatio(i, j) > 0.7){
				registered[j] = true;
				if(windowWithMaxConfidence[currentMaxCarNumber].first < possibleCars[j].confidence){
					auto t = make_pair(possibleCars[j].confidence, j);
					windowWithMaxConfidence[currentMaxCarNumber] = t;
				}
			}
			return 2;
		}
		//No Overlapping : In this case we consider them to belong to different cars.
		else{ 
			return 3;
		}

	}

	/*
		Assigns a car to each entry based on adjacent recatngles. If no information for adjacent entry is present
		new car is assigned to the entry. Right now, there is no updation, i.e. each car is only considered once and
		not updated based on new information.
	*/
	void assignCar(){
		
		for(int i=0; i<(totalCurrentCases-1); i++){
			//Get the releavant Quadcrant
			int currentQuad = checkQuadcrant(i,i+1);
			//If not already registered with any car number
			if(!registered[i]){
				//If its a new car give it new car number
				if(possibleCars[i].car_number == 0){
					possibleCars[i].car_number = ++currentMaxCarNumber;
					//Save the car with max confidence
					auto t = make_pair(possibleCars[i].confidence, i);
					windowWithMaxConfidence.insert({currentMaxCarNumber, t});
				}
				//Check for possible car number
				for(int j=i; j<totalCurrentCases; j++){
					int currentQuad = checkQuadcrant(i,j);
				}
			}
			registered[i] = true;
		}
	}

	void aggregate(){
		//put every entry to specific car
		init();
		assignCar();
	}

	void printTotalCarsDetected(){
		cout << "Total Cars Detected = "<< currentMaxCarNumber << endl;
	}

	void printBestResult(){
		for(int i=1; i<windowWithMaxConfidence.size()+1; i++){
			cout << " i = " << i << " || Car Number " << windowWithMaxConfidence[i].second << "\t";//<< endl;
			printWindow(possibleCars[windowWithMaxConfidence[i].second]);
		}
	}
	/*
		Save output with detected rectangles to "solution/'i'"
	*/
	void saveItToFile(){
		string pathToResult("solution/");
		pathToResult+=to_string(_fileNumber);
		cout << pathToResult << endl;
		std::ofstream file(pathToResult);
		string data;
		// isstringstream iss(data);
		for(int i=1; i<windowWithMaxConfidence.size()+1; i++){
			data = to_string(possibleCars[windowWithMaxConfidence[i].second].x) + " " + 
				   to_string(possibleCars[windowWithMaxConfidence[i].second].y) + " " +
				   to_string(possibleCars[windowWithMaxConfidence[i].second].width) + " " + 
				   to_string(possibleCars[windowWithMaxConfidence[i].second].bredth) + "\n";	
			file << data;
		}
		file.close();

	}

	void applyOnImageAndSave(){

	}

	std::vector<window> possibleCars;
	std::vector<Rect> cars;
	std::vector<bool> registered;
	unordered_map<int, pair<float, int> > windowWithMaxConfidence;

	int currentMaxCarNumber;
	int totalCurrentCases;
	string _fileLocation;
	int _fileNumber;
};

int main()
{
    int files[]={0,1,2,3,4};
    int totalFiles=sizeof(files)/sizeof(*files);

    //Read each file one by one
    for(int i=0; i<totalFiles; i++){
    	cout << "------------------------------------" << endl;

    	//Create Instance for this particular file
    	findCar currentInputFileClass;
    	//Read the input arguments and save inside class member variable
    	currentInputFileClass.readFile(files[i]);
    	//Get total number of arguments passed, just a verification!
    	currentInputFileClass.getStats();
    	//Aggregate function to find the best windows out of given windows
    	currentInputFileClass.aggregate();
    	//Print total cars that are detecte
    	currentInputFileClass.printTotalCarsDetected();
    	//Print the relevant window location which are selected
    	currentInputFileClass.printBestResult();
    	//Save them to file on computer, please create '/solution' folder
    	//before running this function
    	currentInputFileClass.saveItToFile();
    	//Apply the rectanles on the saved image
    	currentInputFileClass.applyOnImageAndSave();
    	cout  << endl;
	}

	cout << "Finished.." << endl;
    return 0;
}