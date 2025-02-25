#include "inventoryobject.h"

#include "../../thirdParty/MBES-lib/src/math/Distance.hpp"

#include "../../thirdParty/MBES-lib/src/utils/Exception.hpp"
#include "../../thirdParty/MBES-lib/src/math/CoordinateTransform.hpp"
#include "../../thirdParty/MBES-lib/src/sidescan/SideScanGeoreferencing.hpp"
#include "../../thirdParty/MBES-lib/src/math/Distance.hpp"

#include <algorithm>

InventoryObject::InventoryObject(SidescanImage & image,int x,int y,int pixelWidth,int pixelHeight,std::string name, std::string description) :
    image(image),
    startPing(*image.getPings().at( y)       ),
    endPing(*image.getPings().at( std::min((int) y+pixelHeight,((int) image.getPings().size()) - 1 )) ),
    x(x),
    y(y),
    pixelWidth(pixelWidth),
    pixelHeight(std::min((int)pixelHeight,(int)image.getPings().size())),
    name(name),
    description(description)
{
    yCenter = y + (pixelHeight/2);
    xCenter = x + (pixelWidth/2);
    
    computeDimensions();
    computePosition();
}

InventoryObject::~InventoryObject(){
    if ( position != nullptr )
        delete position;

}

void InventoryObject::computeDimensions(){
    //FIXME: not as accurate as we could go, ok for ballpark

    if(yCenter < image.getPings().size()){
        width = pixelWidth * image.getPings().at(yCenter)->getDistancePerSample();
    }
    else{
        width = 0;
    }

    if( y < image.getPings().size() && y+pixelHeight < image.getPings().size() ){
        Position * pos1 = image.getPings()[y]->getPosition();
        Position * pos2  = image.getPings()[y+pixelHeight]->getPosition();

        if ( pos1 && pos2  ){
            height = Distance::haversine(pos1->getLongitude(),pos1->getLatitude(),pos2->getLongitude(),pos2->getLatitude());
        }
        else{
            height = std::nan("");
        }
    }
    else{
        height = 0;
    }
}

void InventoryObject::computePosition(){
/*
    if(yCenter < image.getPings().size()){
        SidescanPing * pingCenter = image.getPings()[yCenter];

        //Get start/end pings to compute the direction vector
        SidescanPing * startPing = nullptr;
        SidescanPing * endPing   = nullptr;

        bool reversed = false;

        if(yCenter + 1 < image.getPings().size()){
            //Compute vector using next position
            startPing = image.getPings()[yCenter];
            endPing   = image.getPings()[yCenter+1];
        }
        else if(yCenter - 1 > 0) {
            //No next position, use previous position
            startPing = image.getPings()[yCenter - 1];
            endPing   = image.getPings()[yCenter];
        }
        else {
            //Poop.
            startPing = endPing = pingCenter;
        }

        //Flip if timestamps are inverted
        if(startPing->getTimestamp() > endPing->getTimestamp()){
            SidescanPing * tmp = startPing;
            startPing = endPing;
            endPing   = tmp;
            reversed = true;
            std::cout << "Inverted timestamps" << std::endl;
        }

        Eigen::Vector3d startPositionECEF;
        CoordinateTransform::getPositionECEF(startPositionECEF, *startPing->getPosition());

        Eigen::Vector3d endPositionECEF;
        CoordinateTransform::getPositionECEF(endPositionECEF, *endPing->getPosition());

        Eigen::Vector3d shipDirection = (endPositionECEF - startPositionECEF).normalized();

        Eigen::Vector3d shipPositionEcef;
        CoordinateTransform::getPositionECEF(shipPositionEcef, *pingCenter->getPosition());

        Eigen::Vector3d shipNormal = shipPositionEcef.normalized();

        Eigen::Vector3d starboard = shipDirection.cross(shipNormal);
        Eigen::Vector3d port = -starboard;

        double delta = pingCenter->getDistancePerSample();
        double distanceToObject = delta*yCenter;

        Eigen::Vector3d sideScanDistanceECEF;

        if(image.isStarboard() || (image.isPort() && reversed)) {
            sideScanDistanceECEF = starboard*distanceToObject;
        } else if(image.isPort() || (image.isStarboard() && reversed) ) {
            sideScanDistanceECEF = port*distanceToObject;
        } else {
            position = new Position(
                        pingCenter->getPosition()->getTimestamp(),
                        pingCenter->getPosition()->getLatitude(),
                        pingCenter->getPosition()->getLongitude(),
                        pingCenter->getPosition()->getEllipsoidalHeight());
            return; // this image is neither port nor starboard. For now, use ship position
        }

        // TODO: get this lever arm from platform metadata
        Eigen::Vector3d antenna2TowPoint = image.getFile().getAntenna2TowPointLeverArm();
        Eigen::Matrix3d ship2Ecef;
        ship2Ecef.row(0) = shipDirection;
        ship2Ecef.row(1) = starboard;
        ship2Ecef.row(2) = shipNormal;
        Eigen::Vector3d antenna2TowPointEcef = ship2Ecef*antenna2TowPoint;

        //std::cout << "angle between tangent and normal: " << std::acos(tangentUnitVector.dot(normalUnitVector))*180/M_PI << std::endl;


        // TODO: get this layback from xtf file
        // -tangent vector since tow fish is assumed to be directly behind ship
        // -normal vector since sensorDepth is positive down (according to XTF doc)
        Eigen::Vector3d backEcef = pingCenter->getLayback()*(-shipDirection);
        Eigen::Vector3d downEcef = pingCenter->getSensorDepth()*(-shipNormal);
        Eigen::Vector3d laybackEcef = backEcef + downEcef;

        position = new Position(pingCenter->getTimestamp(), 0.0, 0.0, 0.0);
        //set position of this georeferenced object
        SideScanGeoreferencing::georeferenceSideScanEcef(shipPositionEcef, antenna2TowPointEcef, laybackEcef, sideScanDistanceECEF, *position);
    }
    else{
        position = nullptr;
    }

    if ( position == nullptr ){
        throw new Exception( "GeoreferencedObject::computePosition(): position is nullptr" );
    }
*/
    
    
    if(yCenter < image.getPings().size()){
        SidescanPing * pingCenter = image.getPings()[yCenter];
        
        Eigen::Vector3d shipPositionEcef;
        if(pingCenter->getPosition() == nullptr) {
            throw new Exception("GeoreferencedObject::computePosition(): pingCenter has null Position pointer");
        }
        CoordinateTransform::getPositionECEF(shipPositionEcef, *pingCenter->getPosition());
        
        Eigen::Vector3d tangentUnitVector;
        bool tangentFound = false;
        int offset = 0;

        
        while(!tangentFound) {
            ++offset;
            if(yCenter + offset < image.getPings().size()) {
                SidescanPing * pingAfter = image.getPings()[yCenter+offset];

                if(pingAfter->getTimestamp() < pingCenter->getTimestamp()) {
                    throw new Exception("GeoreferencedObject::computePosition(): pingAfter [yCenter+offset] has lower Timestamp than pingCenter");
                }

                if(Distance::haversine(
                            pingAfter->getPosition()->getLongitude(),
                            pingAfter->getPosition()->getLatitude(),
                            pingCenter->getPosition()->getLongitude(),
                            pingCenter->getPosition()->getLatitude()) > 1.0)
                {
                    Eigen::Vector3d positionAfterECEF;
                    CoordinateTransform::getPositionECEF(positionAfterECEF, *pingAfter->getPosition());
                    tangentUnitVector = (positionAfterECEF - shipPositionEcef).normalized();
                    tangentFound = true;
                }
            } else if(yCenter - offset > 0) {
                SidescanPing * pingBefore = image.getPings()[yCenter - offset];

                if(pingBefore->getTimestamp() > pingCenter->getTimestamp()) {
                    throw new Exception("GeoreferencedObject::computePosition(): pingBefore [yCenter-offset] has greater Timestamp than pingCenter");
                }

                if(Distance::haversine(
                            pingBefore->getPosition()->getLongitude(),
                            pingBefore->getPosition()->getLatitude(),
                            pingCenter->getPosition()->getLongitude(),
                            pingCenter->getPosition()->getLatitude()) > 1.0)
                {
                    Eigen::Vector3d positionBeforeECEF;
                    CoordinateTransform::getPositionECEF(positionBeforeECEF, *pingBefore->getPosition());
                    tangentUnitVector = (shipPositionEcef - positionBeforeECEF).normalized();
                    tangentFound = true;
                }
            } else {
                throw new Exception("GeoreferencedObject::computePosition(): Could not find another position to compute tangent vector to ship trajectory.");
            }
        }
        
        

        Eigen::Vector3d normalUnitVector = shipPositionEcef.normalized();

        Eigen::Vector3d starboardUnitVector = tangentUnitVector.cross(normalUnitVector);
        Eigen::Vector3d portUnitVector = -starboardUnitVector;

        double delta = pingCenter->getDistancePerSample();
        double distanceToObject = delta*yCenter;

        Eigen::Vector3d sideScanDistanceECEF;

        if(image.isStarboard()) {
            sideScanDistanceECEF = starboardUnitVector*distanceToObject;
        } else if(image.isPort()) {
            sideScanDistanceECEF = portUnitVector*distanceToObject;
        } else {
            position = new Position(
                        pingCenter->getPosition()->getTimestamp(),
                        pingCenter->getPosition()->getLatitude(),
                        pingCenter->getPosition()->getLongitude(),
                        pingCenter->getPosition()->getEllipsoidalHeight());
            return; // this image is neither port nor starboard. For now, use ship position
        }

        // TODO: get this lever arm from platform metadata
        Eigen::Vector3d antenna2TowPoint = image.getFile().getAntenna2TowPointLeverArm();
        Eigen::Matrix3d ship2Ecef;
        ship2Ecef.row(0) = tangentUnitVector;
        ship2Ecef.row(1) = starboardUnitVector;
        ship2Ecef.row(2) = normalUnitVector;
        Eigen::Vector3d antenna2TowPointEcef = ship2Ecef*antenna2TowPoint;

        //std::cout << "angle between tangent and normal: " << std::acos(tangentUnitVector.dot(normalUnitVector))*180/M_PI << std::endl;


        // TODO: get this layback from xtf file
        // -tangent vector since tow fish is assumed to be directly behind ship
        // -normal vector since sensorDepth is positive down (according to XTF doc)
        Eigen::Vector3d backEcef = pingCenter->getLayback()*(-tangentUnitVector);
        Eigen::Vector3d downEcef = pingCenter->getSensorDepth()*(-normalUnitVector);
        Eigen::Vector3d laybackEcef = backEcef + downEcef;

        position = new Position(pingCenter->getTimestamp(), 0.0, 0.0, 0.0);
        //set position of this georeferenced object
        
        
        SideScanGeoreferencing::georeferenceSideScanEcef(shipPositionEcef, antenna2TowPointEcef, laybackEcef, sideScanDistanceECEF, *position);
    }
    else{
        position = nullptr;
    }

    if ( position == nullptr )
        throw new Exception( "GeoreferencedObject::computePosition(): position is nullptr" );
}

bool InventoryObject::is_inside(struct region & area){
    /*
    std::cout<<"area:"<<"\n";
    std::cout<<area.x<<" "<<area.y<<" "<<area.width<<" "<<area.height<<"\n";
    std::cout<<"invetory object:"<<"\n";
    std::cout<<this->getX()<<" "<<this->getY()<<" "<<this->getX() + this->getPixelWidth()<<" "<<this->getY() + this->getPixelHeight()<<"\n";
    */
    if(
            (area.x <= this->getX()) && (area.width >= (this->getX() + this->getPixelWidth())) &&
            (area.y <= this->getY()) && (area.height >= (this->getY() + this->getPixelHeight()))
    ){
        return true;
    }
    else{
        return false;
    }
}
