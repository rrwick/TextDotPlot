#include <QCoreApplication>
#include <QString>
#include <QTextStream>
#include <iostream>
#include <vector>
#include <QImage>
#include <math.h>
#include <QSize>
#include <QFile>
#include <QPainter>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("TextDotPlot");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("TextDotPlot");
    parser.addHelpOption();

    parser.addPositionalArgument("source", "Text input filename");
    parser.addPositionalArgument("destination", "Image output filename");
    parser.addPositionalArgument("window", "Window size (number of characters)");

    QCommandLineOption imageSizeOption(QStringList() << "s" << "size",
                                       "Dimension (for both height and width) of the image",
                                       "imageSize", "1000");
    parser.addOption(imageSizeOption);

    QCommandLineOption dotSizeOption(QStringList() << "d" << "dot",
                                     "Diameter of the dots",
                                     "dotDiameter", "2.0");
    parser.addOption(dotSizeOption);

    QCommandLineOption logFileOption(QStringList() << "l" << "log",
                                     "Optional text output log file",
                                     "logFile", "");
    parser.addOption(logFileOption);

    parser.process(app);
    const QStringList args = parser.positionalArguments();
    if (args.size() < 3)
    {
        std::cout << "Not enough arguments provided." << std::endl;
        return 1;
    }

    QString inputFile = args.at(0);
    if (inputFile == "")
    {
        std::cout << "Input file not provided." << std::endl;
        return 1;
    }

    QString outputImage = args.at(1);
    if (outputImage == "")
    {
        std::cout << "Output image not provided." << std::endl;
        return 1;
    }

    int substringSize = args.at(2).toInt();
    if (substringSize <= 0)
    {
        std::cout << "Window size not provided." << std::endl;
        return 1;
    }

    int imageSize = parser.value(imageSizeOption).toInt();
    if (imageSize > 32767 || imageSize <= 0)
    {
        std::cout << "Bad image size." << std::endl;
        return 1;
    }

    double dotSize = parser.value(dotSizeOption).toDouble();
    if (dotSize <= 0.0)
    {
        std::cout << "Bad dot size." << std::endl;
        return 1;
    }

    QString outputText = parser.value(logFileOption);
    bool textFile = (outputText != "");

    QFile file(inputFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        std::cout << "Could not open input file." << std::endl;
        return 1;
    }

    QTextStream in(&file);
    QString wholeFile;
    while(!in.atEnd()) {
        QString line = in.readLine();
        wholeFile += line;
    }
    file.close();
    int wholeFileLength = wholeFile.length();

    QFile logFile(outputText);
    QTextStream outstream;
    if (textFile)
    {
        logFile.open(QFile::WriteOnly);
        outstream.setDevice(&logFile);
    }

    QImage output(imageSize, imageSize, QImage::Format_RGB32);
    QPainter painter(&output);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(0, 0, imageSize, imageSize, Qt::white);
    painter.setBrush(Qt::black);

    double dotRadius = dotSize / 2.0;
    for (int i = 0; i < wholeFileLength - substringSize; ++i)
    {
        double percentDone = 100.0 * double(i) / (wholeFileLength - substringSize);
        std::cout << int(percentDone) << "%    \r";

        QStringRef subString1(&wholeFile, i, substringSize);
        for (int j = i; j < wholeFileLength - substringSize; ++j)
        {
            QStringRef subString2(&wholeFile, j, substringSize);

            if (subString1 == subString2)
            {
                double xPos = double(imageSize) * double(i) / wholeFileLength;
                double yPos = double(imageSize) - (double(imageSize) * double(j) / wholeFileLength);
                painter.drawEllipse(QPointF(xPos, yPos), dotRadius, dotRadius);

                if (i != j)
                {
                    xPos = double(imageSize) * double(j) / wholeFileLength;
                    yPos = double(imageSize) - (double(imageSize) * double(i) / wholeFileLength);
                    painter.drawEllipse(QPointF(xPos, yPos), dotRadius, dotRadius);

                    if (textFile)
                    {
                        QString dupString = wholeFile.mid(i, substringSize);
                        outstream << QString::number(i) << ", " << QString::number(j) << ": " << dupString << endl;
                    }
                }
            }
        }
    }

    output.save(outputImage);
    logFile.close();
    std::cout << "100%    " << std::endl;
}
