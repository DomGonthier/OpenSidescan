#include "project.h"

#include <cstring>
#include <vector>

#include <QFile>
#include <QtXml>
#include <QPixmap>

#include "sidescan/sidescanimager.h"
#include "utilities/qthelper.h"





Project::Project()
{

}

Project::~Project(){
    mutex.lock();

    for(auto i=files.begin();i!=files.end();i++){
        delete (*i);
    }

    mutex.unlock();
}

void Project::addFile(SidescanFile * newFile){
    mutex.lock();

    files.push_back(newFile);

    mutex.unlock();
}

void Project::removeFile(SidescanFile * file){
    mutex.lock();

    auto iter = std::find( files.begin(), files.end(), file );

    if ( iter != files.end() )
    {
        files.erase( iter );
    }

    mutex.unlock();
}

void Project::read(std::string & filename){
    QFile file(QString::fromStdString(filename));
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader xml(&file);

    mutex.lock();

    std::string currentImage;
    SidescanFile * currentFile= nullptr;

    while(!xml.atEnd()){

        //Read through file
        switch(xml.readNext()){
            case QXmlStreamReader::StartElement:
                std::string name = xml.name().toString().toStdString();

                //Handle different element types
                if(strncmp(name.c_str(),"Project",7)==0) {
                    //read lever arm
                    QStringRef leverArmX = xml.attributes().value(QString::fromStdString("leverArmX"));
                    QStringRef leverArmY = xml.attributes().value(QString::fromStdString("leverArmY"));
                    QStringRef leverArmZ = xml.attributes().value(QString::fromStdString("leverArmZ"));

                    if(leverArmX.isEmpty() || leverArmY.isEmpty() || leverArmZ.isEmpty() ) {
                        //no lever arm, old project file
                        antenna2TowPointLeverArm << 0 ,0 ,0;
                    } else {
                        antenna2TowPointLeverArm << leverArmX.toDouble(), leverArmY.toDouble(), leverArmZ.toDouble();
                    }
                } else if(strncmp(name.c_str(),"File",4)==0){
                    //Sidescan file
                    std::string filename = xml.attributes().value(QString::fromStdString("filename")).toString().toStdString();

                    SidescanImager imager;
                    DatagramParser * parser = DatagramParserFactory::build(filename,imager);
                    parser->parse(filename);
                    currentFile = imager.generate(filename, antenna2TowPointLeverArm);

                    files.push_back(currentFile);

                    currentImage = "";

                    delete parser;
                }
                else if(strncmp(name.c_str(),"Image",5)==0){
                    //Sidescan image
                    currentImage=xml.attributes().value(QString::fromStdString("channelName")).toString().toStdString();
                }
                else if(strncmp(name.c_str(),"Object",5)==0){
                    //Inventory Objects
                    if(currentFile){
                        for(auto i = currentFile->getImages().begin();i!=currentFile->getImages().end();i++){
                            if(strncmp((*i)->getChannelName().c_str(),currentImage.c_str(),currentImage.size())==0){
                                //instanciate object
                                int x                   = std::stoi(xml.attributes().value(QString::fromStdString("x")).toString().toStdString());
                                int y                   = std::stoi(xml.attributes().value(QString::fromStdString("y")).toString().toStdString());
                                int pixelWidth          = std::stoi(xml.attributes().value(QString::fromStdString("pixelWidth")).toString().toStdString());
                                int pixelHeight         = std::stoi(xml.attributes().value(QString::fromStdString("pixelHeight")).toString().toStdString());
                                std::string name        = xml.attributes().value(QString::fromStdString("name")).toString().toStdString();
                                std::string description = xml.attributes().value(QString::fromStdString("description")).toString().toStdString();

                                InventoryObject * object = new InventoryObject(*(*i),x,y,pixelWidth,pixelHeight,name,description);
                                (*i)->getObjects().push_back(object);
                            }
                        }

                    }
                    else{
                        //No file...wtf
                        std::cerr << "Malformed Project File: No file associated with object" << std::endl;
                    }
                }
            break;
        }
    }

    mutex.unlock();
}

void Project::write(std::string & filename){
    mutex.lock();

    QFile file(QString::fromStdString(filename));
    file.open(QIODevice::WriteOnly);

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("Project");
    xmlWriter.writeAttribute(QString::fromStdString("leverArmX"),QString::number(antenna2TowPointLeverArm[0]));
    xmlWriter.writeAttribute(QString::fromStdString("leverArmY"),QString::number(antenna2TowPointLeverArm[1]));
    xmlWriter.writeAttribute(QString::fromStdString("leverArmZ"),QString::number(antenna2TowPointLeverArm[2]));

    //prepare to compute relative file paths
    QFileInfo fileInfo(QCoreApplication::applicationFilePath());
    QDir projectDir(fileInfo.canonicalPath());
    //qDebug()<<fileInfo.canonicalPath();

    for(auto i=files.begin();i!=files.end();i++){
        xmlWriter.writeStartElement("File");

        QString sssRelativePath = projectDir.relativeFilePath(QString::fromStdString((*i)->getFilename()));

        //qDebug()<<sssRelativePath<<"\n";

        xmlWriter.writeAttribute(QString::fromStdString("filename"),sssRelativePath);

        for(auto j=(*i)->getImages().begin();j!=(*i)->getImages().end();j++){
            //TODO: write objects
            xmlWriter.writeStartElement("Image");

            xmlWriter.writeAttribute(QString::fromStdString("channelName"), QString::fromStdString((*j)->getChannelName()) );

            for(auto k = (*j)->getObjects().begin(); k != (*j)->getObjects().end(); k++){
                xmlWriter.writeStartElement("Object");

                xmlWriter.writeAttribute(QString::fromStdString("x"),           QString::fromStdString( std::to_string((*k)->getX())            ) );
                xmlWriter.writeAttribute(QString::fromStdString("y"),           QString::fromStdString( std::to_string((*k)->getY())            ) );
                xmlWriter.writeAttribute(QString::fromStdString("pixelWidth"),  QString::fromStdString( std::to_string((*k)->getPixelWidth())   ) );
                xmlWriter.writeAttribute(QString::fromStdString("pixelHeight"), QString::fromStdString( std::to_string((*k)->getPixelHeight())  ) );
                xmlWriter.writeAttribute(QString::fromStdString("name"),        QString::fromStdString( (*k)->getName()                         ) );
                xmlWriter.writeAttribute(QString::fromStdString("description"), QString::fromStdString( (*k)->getDescription()                  ) );

                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndElement();
        }

        xmlWriter.writeEndElement();
    }

    xmlWriter.writeEndElement();

    file.close();

    mutex.unlock();
}


void Project::exportInventoryAsKml(std::string & filename){
    QFile file(QString::fromStdString(filename));
    file.open(QIODevice::WriteOnly);

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("kml");
    xmlWriter.writeNamespace(QString::fromStdString("http://www.opengis.net/kml/2.2"));
    xmlWriter.writeStartElement("Document");

    mutex.lock();

    for(auto i=files.begin();i!=files.end();i++){
        for(auto j=(*i)->getImages().begin();j!=(*i)->getImages().end();j++){
            for(auto k=(*j)->getObjects().begin();k!=(*j)->getObjects().end();k++){

                if((*k)->getPosition()){
                    xmlWriter.writeStartElement("Placemark");

                    //name
                    xmlWriter.writeStartElement("name");
                    xmlWriter.writeCharacters(QString::fromStdString((*k)->getName()));
                    xmlWriter.writeEndElement();

                    //description
                    xmlWriter.writeStartElement("description");
                    xmlWriter.writeCDATA(QString::fromStdString((*k)->getDescription()));
                    xmlWriter.writeEndElement();

                    //Point coordinates
                    std::stringstream ss;
                    ss << std::setprecision(15);
                    ss << (*k)->getPosition()->getLongitude() << "," << (*k)->getPosition()->getLatitude() ;

                    xmlWriter.writeStartElement("Point");

                    xmlWriter.writeStartElement("coordinates");
                    xmlWriter.writeCharacters(QString::fromStdString(ss.str()));
                    xmlWriter.writeEndElement();

                    xmlWriter.writeEndElement();


                    xmlWriter.writeEndElement();
                }
            }
        }
    }

    mutex.unlock();

    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();

    file.close();
}

void Project::exportInventoryAsCsv(std::string & filename){
    std::ofstream outFile;
    outFile.open( filename, std::ofstream::out );

    if( outFile.is_open() ){
        outFile << "name" << "," << "description" << "," << "longitude" << "," << "latitude" << "\n";

        outFile << std::fixed << std::setprecision(15);

        mutex.lock();

        for(auto i = files.begin(); i != files.end(); ++i){
            for(auto j=(*i)->getImages().begin();j!=(*i)->getImages().end();j++){
                for(auto k=(*j)->getObjects().begin();k!=(*j)->getObjects().end();k++){
                    Position * pos = (*k)->getPosition();
                    outFile << "\"" << (*k)->getName() << "\"," << "\"" << (*k)->getDescription() << "\","  << pos->getLongitude() << "," << pos->getLatitude() << "\n";
                }
            }
        }

        mutex.unlock();

        outFile.close();
    }
}

void Project::exportInventoryAsHits(std::string & path){

    for(auto i = files.begin(); i != files.end(); ++i){

        std::string filename = (*i)->getFilename();
        QFileInfo fileInfo(QString::fromStdString(filename));
        QString FileName = fileInfo.fileName();
        QFileInfo pathInfo(QString::fromStdString(path));
        pathInfo.setFile(QString::fromStdString(path),FileName);
        QString filePath = pathInfo.filePath();
        std::string Path = filePath.toStdString();
        Path.append(".hits");

        std::ofstream outFile;
        outFile.open( Path, std::ofstream::out );
        if( outFile.is_open() ){
            mutex.lock();
            for(auto j=(*i)->getImages().begin();j!=(*i)->getImages().end();j++){
                for(auto k=(*j)->getObjects().begin();k!=(*j)->getObjects().end();k++){
                    outFile<<(*j)->getChannelNumber()<< " "   << (*k)->getXCenter() << " " << (*k)->getYCenter() << "\n";
                }
            }
            mutex.unlock();
            outFile.close();
            Path = "";
        }
        else{
            std::cerr<<"cant create new file"<<std::endl;
        }
    }
}


//void Project::saveObjectImages( const QString & folder )
void Project::saveObjectImages( const QString & absolutePath, const QString & fileNameWithoutExtension )
{
    QString fileNameHTML = absolutePath + "/" + fileNameWithoutExtension + ".html";

    QFile file( fileNameHTML );

    if( file.open(QIODevice::WriteOnly) )
    {
        QXmlStreamWriter xmlWriter(&file);

        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();

        xmlWriter.writeDTD( "<!DOCTYPE html>" );

        xmlWriter.writeStartElement("html");

        // Style

        xmlWriter.writeStartElement("head");
        xmlWriter.writeStartElement("style");

        xmlWriter.writeCharacters( "table, th, td {\n" );
        xmlWriter.writeCharacters( "  border: 1px solid black;\n" );
        xmlWriter.writeCharacters( "  border-collapse: collapse;\n" );
        xmlWriter.writeCharacters( "}\n" );
        xmlWriter.writeCharacters( "th, td {\n" );
        xmlWriter.writeCharacters( "  padding: 5px;\n" );
        xmlWriter.writeCharacters( "}\n" );
        xmlWriter.writeCharacters( "th {\n" );
        xmlWriter.writeCharacters( "  text-align: left;\n" );
        xmlWriter.writeCharacters( "}\n" );

        xmlWriter.writeEndElement(); // style
        xmlWriter.writeEndElement(); // head

        // Body

        xmlWriter.writeStartElement("body");

        xmlWriter.writeStartElement("h2"); // Left-align Headings
        xmlWriter.writeCharacters( "Objects" );
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("p");
        xmlWriter.writeCharacters( "List of objects" );
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement( "table style=\"width:100%\"" );

        // Table header
        xmlWriter.writeStartElement("tr");

        xmlWriter.writeStartElement("th");
        xmlWriter.writeCharacters( "Target" );
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("th");
        xmlWriter.writeCharacters( "Name" );
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("th");
        xmlWriter.writeCharacters( "File" );
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("th");
        xmlWriter.writeCharacters( "Channel" );
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("th");
        xmlWriter.writeCharacters( "Longitude" );
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("th");
        xmlWriter.writeCharacters( "Latitude" );
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("th");
        xmlWriter.writeCharacters( "Width (m)" );
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("th");
        xmlWriter.writeCharacters( "Height (m)" );
        xmlWriter.writeEndElement();


        xmlWriter.writeEndElement(); // tr

        mutex.lock();

        for(auto i = files.begin(); i != files.end(); ++i){

            for(auto j=(*i)->getImages().begin();j!=(*i)->getImages().end();j++){

                for(auto k=(*j)->getObjects().begin();k!=(*j)->getObjects().end();k++){

                    // Copy the part of the cv::Mat with the object into a new cv::Mat
                    cv::Mat objectMat ( (*k)->getPixelHeight() ,(*k)->getPixelWidth(), CV_8UC1 );
                    (*j)->getImage()( cv::Rect( (*k)->getX(), (*k)->getY(), (*k)->getPixelWidth(), (*k)->getPixelHeight() ) ).copyTo( objectMat );

                    // Find filename that does not already exist
                    QString objectName = QString::fromStdString( (*k)->getName() );

                    QString fileExtension = "png";

                    QString objectImageFileName = objectName + "." + fileExtension;

                    QString objectImageFileNameWithPath = absolutePath + "/" + fileNameWithoutExtension + "/" + objectImageFileName;

                    QFileInfo fileInfo( objectImageFileNameWithPath );

                    int count = 0;

                    while ( fileInfo.exists() ) {

                        objectImageFileName = objectName + "_" + QString::number( count ) + "." + fileExtension;
                        objectImageFileNameWithPath = absolutePath + "/" + fileNameWithoutExtension + "/" + objectImageFileName;
                        fileInfo.setFile( objectImageFileNameWithPath );
                        count++;
                    }

                    // Save pixmap
                    cv::imwrite(objectImageFileNameWithPath.toStdString(),objectMat);

                    xmlWriter.writeStartElement("tr");

                    //Image
                    xmlWriter.writeStartElement("td");

                    QString imageString = "img src=\"" + fileNameWithoutExtension + "/" + objectImageFileName + "\" alt=\"" + objectImageFileName + "\"";
                    xmlWriter.writeStartElement( imageString );
                    xmlWriter.writeEndElement(); // imageString

                    xmlWriter.writeEndElement(); // td

                    //Target name

                    xmlWriter.writeStartElement("td");
                    xmlWriter.writeCharacters( objectName );
                    xmlWriter.writeEndElement();

                    //File name
                    QFileInfo sidescanFileInfo( QString::fromStdString((*i)->getFilename()) );
                    QString filenameWithoutPath = sidescanFileInfo.fileName();

                    xmlWriter.writeStartElement("td");
                    xmlWriter.writeCharacters( filenameWithoutPath );
                    xmlWriter.writeEndElement();

                    //Channel
                    xmlWriter.writeStartElement("td");
                    xmlWriter.writeCharacters(  QString::fromStdString((*j)->getChannelName()) );
                    xmlWriter.writeEndElement();

                    //Longitude / Latitude

                    xmlWriter.writeStartElement("td");
                    xmlWriter.writeCharacters( ((*k)->getPosition()) ? QString::number((*k)->getPosition()->getLongitude(), 'f', 15) : "N/A" );
                    xmlWriter.writeEndElement();

                    xmlWriter.writeStartElement("td");
                    xmlWriter.writeCharacters( ((*k)->getPosition()) ? QString::number((*k)->getPosition()->getLatitude(), 'f', 15) : "N/A" );
                    xmlWriter.writeEndElement();

                    //Width
                    xmlWriter.writeStartElement("td");
                    xmlWriter.writeCharacters( ((*k)->getWidth() > 0) ?  QString::number( (*k)->getWidth(), 'f', 3) : "N/A" );
                    xmlWriter.writeEndElement();

                    //Height
                    xmlWriter.writeStartElement("td");
                    xmlWriter.writeCharacters( ((*k)->getHeight() > 0) ? QString::number( (*k)->getHeight(), 'f', 3) : "N/A" );
                    xmlWriter.writeEndElement();

                    xmlWriter.writeEndElement(); // tr
                }
            }
        }

        mutex.unlock();

        xmlWriter.writeEndDocument(); // Closes all remaining open start elements and writes a newline.

        file.close();
    }

    mutex.unlock();
}

unsigned long Project::getFileCount()
{
    unsigned long count = 0;

    mutex.lock();

    count = files.size();

    mutex.unlock();

    return count;
}

unsigned long Project::getObjectCount()
{
    unsigned long count=0;

    mutex.lock();

    for(auto i = files.begin();i != files.end();i++){
        for(auto j = (*i)->getImages().begin(); j != (*i)->getImages().end();j++){
            count += (*j)->getObjects().size();
        }
    }

    mutex.unlock();

    return count;
}




bool Project::containsFile(std::string & filename){
    bool res = false;

    mutex.lock();

    for(auto i=files.begin();i!=files.end();i++){
        if(strcmp((*i)->getFilename().c_str(),filename.c_str()) == 0){
            res=true;
            break;
        }
    }

    mutex.unlock();

    return res;
}

void Project::exportInventory4Yolo(std::string & path){
    for(auto i = files.begin(); i != files.end(); ++i){      
        for(auto j=(*i)->getImages().begin();j!=(*i)->getImages().end();j++){
            int image_count =0;
            for(unsigned int index = 0; index < (*j)->getObjects().size(); index++){
                auto k = (*j)->getObjects().at(index);
                std::string filename = (*i)->getFilename();             //get file name
                QFileInfo fileInfo(QString::fromStdString(filename));
                QString FileName = fileInfo.fileName();
                QFileInfo pathInfo(QString::fromStdString(path));
                pathInfo.setFile(QString::fromStdString(path),FileName);
                QString filePath = pathInfo.filePath();
                std::string FILEPATH = filePath.toStdString();

                QString chan = QString::number((*j)->getChannelNumber());  //get channel number
                std::string channel = chan.toStdString();
                image_count ++;                                             //image number
                QString ImageCount = QString::number(image_count);
                std::string count = ImageCount.toStdString();

                FILEPATH.append("-" + channel + "_" + count);
                std::string image_name = FILEPATH + ".jpg";  //final image name
                FILEPATH.append(".txt");                    //final hits file name

                cv::Mat image = (*j)->getImage();                           //get image
                cv::Size image_dimension = image.size();
                int height = image_dimension.height;                        //get image height
                int width = image_dimension.width;
                int start_range_height = 0;
                int end_range_height = 0;

                //cropping selection
                if(width < height){
                    start_range_height = k->getY() - width/2 ;
                    end_range_height = k->getY() + width/2;

                    //handle cropping selection execptions
                    if(start_range_height < 0 || end_range_height > height ){

                        if(start_range_height < 0 ){
                            end_range_height = width;
                            start_range_height = 0;
                        }
                        if(end_range_height > height){
                            start_range_height = height - width;
                            end_range_height = height;
                        }
                    }
                }
                else{
                    end_range_height = image_dimension.height;
                    int padding = image_dimension.width - image_dimension.height;
                    cv::copyMakeBorder( image, image, 0, padding, 0, 0, cv::BORDER_CONSTANT);
                    image_dimension = image.size();
                }

                if(end_range_height - start_range_height != image_dimension.width){
                    if(image_dimension.width - (end_range_height - start_range_height) > 0){
                        end_range_height += (image_dimension.width - (end_range_height - start_range_height));
                    }
                    else{
                        end_range_height -= ((end_range_height - start_range_height) - image_dimension.width );
                    }
                }

                image = image(cv::Range(start_range_height, end_range_height), cv::Range(0,image_dimension.width));
                cv::imwrite(image_name,image);

                std::ofstream outFile;
                outFile.open( FILEPATH, std::ofstream::out );
                if( outFile.is_open() ){
                    mutex.lock();

                    // All element in cropping section gets written to same file
                    //not handling partial bounding box
                    struct region crop_image{0,start_range_height,image_dimension.width,end_range_height};
                    int inside_count = 0;
                    for(unsigned int index2 = index; index2 < (*j)->getObjects().size(); index2++){
                        auto obj = (*j)->getObjects().at(index2);
                        if(obj->is_inside(crop_image) == true){
                            //std::cout<<"is inside \n";
                            inside_count++;
                            /*
                            One row per object
                            Each row is class x_center y_center width height format.
                            Box coordinates must be in normalized xywh format (from 0 - 1).
                            If your boxes are in pixels, divide x_center and width by image width, and y_center and height by image height.
                            */

                            // images are dimension [width X width] , to normalise we divide by [width X width]
                            double norm_detect_xCenter = double((obj->getXCenter()/double(width)));
                            double norm_detect_yCenter = double((double(obj->getPixelHeight())/2.0)/double(width));
                            double detect_norm_width = double((obj->getPixelWidth()/double(width)));
                            double detect_norm_height = double(double(obj->getPixelHeight())/double(width));
                            //for debugging purposes
                            /*
                            int norm_detect_xCenter = obj->getXCenter();
                            int norm_detect_yCenter = obj->getYCenter();
                            int detect_norm_width = obj->getPixelWidth();
                            int detect_norm_height = obj->getPixelHeight();
                            */

                            //Hardcoded class
                            //class map could be build by reading inventory obj
                            std::map<std::string,int> CLASS { {"crabtrap", 0}, {"rope", 1}, {"shipwreck", 2}, };
                            auto search = CLASS.find(obj->getName()); //object inventory name is class name
                            int Class = 0;
                            if (search != CLASS.end()) {
                                    //std::cout << "Found " << search->first << " " << search->second << '\n';
                                    Class = search->second;
                                } else {
                                    //std::cout << "Not found\n";
                                    Class = CLASS.size() + 1;
                                }

                            outFile<< Class <<" "<< norm_detect_xCenter <<" "<< norm_detect_yCenter <<" "
                                << detect_norm_width <<" "<< detect_norm_height <<"\n";

                        }
                    }
                    index += inside_count - 1;
                }
                else{
                    std::cerr<<"cant create new file"<<std::endl;
                }
                mutex.unlock();
                outFile.close();
            }
        }
    }
}
