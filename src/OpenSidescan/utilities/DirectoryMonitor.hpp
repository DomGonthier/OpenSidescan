/*
 * Copyright 2020 © Centre Interdisciplinaire de développement en Cartographie des Océans (CIDCO), Tous droits réservés
 */

/* 
 * File:   DirectoryMonitor.hpp
 * Author: Jordan McManus
 */

#ifndef DIRECTORYMONITOR_HPP
#define DIRECTORYMONITOR_HPP

#include <experimental/filesystem>

//need g++ 8 to remove experimental namespace
namespace fs = std::experimental::filesystem;

class DirectoryMonitor {
public:

    DirectoryMonitor(Detector * detector, SideScanFileProcessor * processor, Eigen::Vector3d leverArm) : detector(detector), processor(processor), leverArm(leverArm) {
    };

    void monitor(std::string path) {
        for (const auto & entry : fs::directory_iterator(path)) {
            std::string filepath = entry.path().generic_string();

            if (FileLockUtils::fileNotLocked(filepath)) {
                if (isXtf(entry)) {
                    if (!alreadyScanned(filepath)) {
                        std::string filepath = entry.path().generic_string();

                        processor->reportProgress("Loading and detecting objects in file " + filepath);
                        SidescanFile * file = loadAndDetectObjects(filepath);
                        processor->processFile(file);
                        processor->reportProgress("Objects detected in file " + filepath);

                        scannedFiles.insert(std::pair<std::string, std::string>(filepath, filepath));
                    } else {
                        //File is already scanned, do nothing
                    }
                }
            } else {
                //File is locked, do nothing
                processor->reportProgress("File is locked by another program: " + filepath);
            }
        }
    }

    SidescanFile * loadAndDetectObjects(std::string & filepath) {
        SidescanImager imager;
        DatagramParser * parser = NULL;
        parser = DatagramParserFactory::build(filepath, imager);
        parser->parse(filepath);

        SidescanFile * file = imager.generate(filepath, leverArm);

        for (auto j = file->getImages().begin(); j != file->getImages().end(); j++) {
            detector->detect(**j, (*j)->getObjects());
        }

        return file;
    }

    void setAlreadyScanned(std::vector<std::string> alreadyScanned) {
        for (unsigned int i = 0; i < alreadyScanned.size(); i++) {
            scannedFiles.insert(std::pair<std::string, std::string>(alreadyScanned[i], alreadyScanned[i]));
        }
    }

private:

    Detector * detector;
    Eigen::Vector3d leverArm;
    SideScanFileProcessor * processor;
    std::map<std::string, std::string> scannedFiles;

    bool isXtf(const fs::directory_entry & entry) {
        //eventually can scan other file extensions
        return entry.path().extension() == ".xtf";
    }

    bool alreadyScanned(std::string & filepath) {
        return scannedFiles.count(filepath);
    }

};


#endif /* DIRECTORYMONITOR_HPP */

