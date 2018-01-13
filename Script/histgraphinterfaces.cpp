#include "histgraphinterfaces.h"
#include "tmpobjhubclass.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

#include "TObject.h"
#include "TH1D.h"
#include "TH1.h"
#include "TH2D.h"
#include "TH2.h"
#include "TF2.h"
#include "TGraph.h"
#include "TF1.h"
#include "TStyle.h"
#include "TAttFill.h"
#include "THStack.h"

//----------------- HIST  -----------------
AInterfaceToHist::AInterfaceToHist(TmpObjHubClass* TmpHub)
  : TmpHub(TmpHub)
{
    H["Draw"] = "Draw the histogram. The second arguments defines how it will be drawn.\nFor available options see:\n"
                "https://root.cern.ch/doc/master/classTHistPainter.html";
    H["FitGauss"] = "Fit histogram with a Gaussian. The returned result (is successful) contains an array [Constant,Mean,Sigma,ErrConstant,ErrMean,ErrSigma]"
            "\nOptional 'options' parameter is directly forwarded to TH1::Fit()";
    H["FitGaussWithInit"] = "Fit histogram with a Gaussian. The returned result (is successful) contains an array [Constant,Mean,Sigma,ErrConstant,ErrMean,ErrSigma]"
                            "\nInitialParValues is an array of initial parameters of the values [Constant,Mean,Sigma]"
                            "\nOptional 'options' parameter is directly forwarded to TH1::Fit()";
    H["SetOptStat"] =
            "k : kurtosis printed\n"
            "K : kurtosis and kurtosis error printed\n"
            "s : skewness printed\n"
            "S : skewness and skewness error printed\n"
            "i : integral of bins printed\n"
            "I : integral of bins with option \"width\" printed\n"
            "o : number of overflows printed\n"
            "u : number of underflows printed\n"
            "r : rms printed\n"
            "R : rms and rms error printed\n"
            "m : mean value printed\n"
            "M : mean value mean error values printed\n"
            "e : number of entries printed\n"
            "n : name of histogram is printed\n"
            "Example: SetOptStat(\"ne\"); print only name of histogram and number of entries.";
}

bool AInterfaceToHist::InitOnRun()
{
  //TmpHub->ScriptDrawObjects.clear();
  return true;
}

void AInterfaceToHist::NewHist(QString HistName, int bins, double start, double stop)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
  if (index != -1)
    {
      abort("Bad new hist name! Object "+HistName+" already exists");
      return;
    }

  TH1D* hist = new TH1D("", HistName.toLatin1().data(), bins, start, stop);
  hist->GetYaxis()->SetTitleOffset((Float_t)1.30);
  hist->SetOption("HIST");
  TmpHub->ScriptDrawObjects.append(hist, HistName, "TH1D");
}

void AInterfaceToHist::NewHist2D(QString HistName, int binsX, double startX, double stopX, int binsY, double startY, double stopY)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
  if (index != -1)
    {
      abort("Bad new hist name! Object "+HistName+" already exists");
      return;
    }

  TH2D* hist = new TH2D("", HistName.toLatin1().data(), binsX, startX, stopX, binsY, startY, stopY);
  hist->GetYaxis()->SetTitleOffset((Float_t)1.30);
  TmpHub->ScriptDrawObjects.append(hist, HistName, "TH2D");
}

void AInterfaceToHist::SetTitles(QString HistName, QString X_Title, QString Y_Title, QString Z_Title)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
  if (index == -1)
    {
      abort("Histogram "+HistName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  r.Xtitle = X_Title;
  r.Ytitle = Y_Title;
  r.Ztitle = Z_Title;
}

void AInterfaceToHist::SetLineProperties(QString HistName, int LineColor, int LineStyle, int LineWidth)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
  if (index == -1)
    {
      abort("Histogram "+HistName+" not found!");
      return;
    }

//  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
//  r.LineColor = LineColor;
//  r.LineStyle = LineStyle;
//  r.LineWidth = LineWidth;

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  if (r.type.startsWith("TH"))
    {
      TH1* h = dynamic_cast<TH1*>(r.Obj);
      if (h)
      {
          //qDebug() << "Hereeeeeee!";
          h->SetLineColor(LineColor);
          h->SetLineStyle(LineStyle);
          h->SetLineWidth(LineWidth);
      }
    }
}

void AInterfaceToHist::SetFillProperties(QString HistName, int color, int style)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
    if (index == -1)
      {
        abort("Histogram "+HistName+" not found!");
        return;
      }

    RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
    if (r.type.startsWith("TH"))
      {
        TH1* h = dynamic_cast<TH1*>(r.Obj);
        if (h)
        {
            //qDebug() << "Setting filling";
            h->SetFillColor(color);
            h->SetFillStyle(style);
        }
      }

}

void AInterfaceToHist::Fill(QString HistName, double val, double weight)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
  if (index == -1)
    {
      abort("Histogram "+HistName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  if (r.type == "TH1D")
    {
      TH1D* h = static_cast<TH1D*>(r.Obj);
      h->Fill(val, weight);
    }
  else
    {
      abort("TH1D histogram "+HistName+" not found!");
      return;
    }
}

void AInterfaceToHist::Fill2D(QString HistName, double x, double y, double weight)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
  if (index == -1)
    {
      abort("Histogram "+HistName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  if (r.type == "TH2D")
    {
      TH2D* h = static_cast<TH2D*>(r.Obj);
      h->Fill(x, y, weight);
    }
  else
    {
      abort("TH2D histogram "+HistName+" not found!");
      return;
  }
}

void AInterfaceToHist::FillArr(QString HistName, QVariant Array)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
    if (index == -1)
      {
        abort("Histogram "+HistName+" not found!");
        return;
      }
    RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
    if (r.type != "TH1D")
    {
        abort("This is not a 1D histogram!");
        return;
    }
    TH1D* h = static_cast<TH1D*>(r.Obj);

    QString typeArr = Array.typeName();
    if (typeArr != "QVariantList")
    {
        abort("Array is expected as second argument in hist.FillArr");
        return;
    }

    QVariantList vl = Array.toList();
    QJsonArray arr = QJsonArray::fromVariantList(vl);
    for (int i=0; i<arr.size(); i++)
    {
        if (arr[i].isDouble()) h->Fill(arr[i].toDouble(), 1.0);
        else
        {
            QJsonArray el = arr[i].toArray();
            double val, weight;
            switch (el.size())
            {
            case 1:
                weight = 1.0;
                break;
            case 2:
                weight = el[1].toDouble();
                break;
            default:
                abort("hist.FillArr: second argument has to be array of values (then weight=1) or of arrays of [val, weight]");
                return;
            }
            val = el[0].toDouble();

            h->Fill(val, weight);
        }
    }
}

void AInterfaceToHist::Fill2DArr(QString HistName, QVariant Array)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
    if (index == -1)
      {
        abort("Histogram "+HistName+" not found!");
        return;
      }
    RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
    if (r.type != "TH2D")
    {
        abort("This is not a 2D histogram!");
        return;
    }
    TH2D* h = static_cast<TH2D*>(r.Obj);

    QString typeArr = Array.typeName();
    if (typeArr != "QVariantList")
    {
        abort("Array is expected as second argument in hist.Fill2DArr");
        return;
    }

    QVariantList vl = Array.toList();
    QJsonArray arr = QJsonArray::fromVariantList(vl);
    for (int i=0; i<arr.size(); i++)
    {
        if (!arr[i].isArray())
        {
            abort("Fill2DArr expect the second argument to be array of arrays [x, y] (or [x, y, weight])");
            return;
        }
        else
        {
            QJsonArray el = arr[i].toArray();
            double x, y, weight;
            switch (el.size())
            {
            case 2:
                weight = 1.0;
                break;
            case 3:
                weight = el[2].toDouble();
                break;
            default:
                abort("hist.FillArr: second argument has to be array of values (then weight=1) or of arrays of [val, weight]");
                return;
            }
            x = el[0].toDouble();
            y = el[1].toDouble();

            h->Fill(x, y, weight);
        }
    }
}

void AInterfaceToHist::Divide(QString HistName, QString HistToDivideWith)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
    if (index == -1)
      {
        abort("Histogram "+HistName+" not found!");
        return;
      }
    RootDrawObj& r1 = TmpHub->ScriptDrawObjects.List[index];
    if (!r1.type.startsWith("TH"))
    {
        abort("Histogram "+HistName+" not found!");
        return;
    }

    index = TmpHub->ScriptDrawObjects.findIndexOf(HistToDivideWith);
    if (index == -1)
      {
        abort("Histogram "+HistToDivideWith+" not found!");
        return;
      }
    RootDrawObj& r2 = TmpHub->ScriptDrawObjects.List[index];
    if (!r2.type.startsWith("TH"))
    {
        abort("Histogram "+HistToDivideWith+" not found!");
        return;
    }

    TH1* h1 = dynamic_cast<TH1*>(r1.Obj);
    if (h1)
    {
        TH1* h2 = dynamic_cast<TH1*>(r2.Obj);
        if (h2)
        {
            bool bOK = h1->Divide(h2);
            if (bOK) return;
        }
    }
    abort("Division failed!");
}

void AInterfaceToHist::Smooth(QString HistName, int times)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
  if (index == -1)
    {
      abort("Histogram "+HistName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  if (r.type == "TH1D")
    {
      TH1D* h = static_cast<TH1D*>(r.Obj);
      h->Smooth(times);
      emit RequestDraw(0, "", true); //to update
    }
  else if (r.type == "TH2D")
    {
      TH2D* h = static_cast<TH2D*>(r.Obj);
      h->Smooth(times);
      emit RequestDraw(0, "", true); //to update
    }
  else
    {
      abort("Object "+HistName+": unknown histogram type!");
      return;
  }
}

QVariant ReturnNanArray(int num)
{
    QJsonArray ar;
    for (int i=0; i<num; i++) ar << std::numeric_limits<double>::quiet_NaN();
    QJsonValue jv = ar;
    QVariant res = jv.toVariant();
    return res;
}

QVariant AInterfaceToHist::FitGauss(QString HistName, QString options)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
    if (index == -1)
      {
        abort("Histogram "+HistName+" not found!");
        return ReturnNanArray(6);
      }

    RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
    if (r.type.startsWith("TH1"))
      {
        TH1* h = static_cast<TH1*>(r.Obj);
        TF1 *f1 = new TF1("f1", "gaus");
        int status = h->Fit(f1, options.toLatin1());
        if (status != 0) return ReturnNanArray(6);

        emit RequestDraw(0, "", true); //to update

        QJsonArray ar;
        for (int i=0; i<3; i++) ar << f1->GetParameter(i);
        for (int i=0; i<3; i++) ar << f1->GetParError(i);

        QJsonValue jv = ar;
        QVariant res = jv.toVariant();
        return res;
      }
    else
      {
        abort("Object "+HistName+": unsupported histogram type!");
        return ReturnNanArray(6);
    }
}

QVariant AInterfaceToHist::FitGaussWithInit(QString HistName, QVariant InitialParValues, QString options)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
    if (index == -1)
      {
        abort("Histogram "+HistName+" not found!");
        return ReturnNanArray(6);
      }

    QString type = InitialParValues.typeName();
    if (type != "QVariantList")
    {
        abort("InitialParValues has to be an array of three numeric values");
        return ReturnNanArray(6);
    }

    QVariantList vl = InitialParValues.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);
    if (ar.size() < 3)
    {
        abort("InitialParValues has to be an array of three numeric values");
        return ReturnNanArray(6);
    }
    if (!ar[0].isDouble() || !ar[1].isDouble() || !ar[2].isDouble() )
    {
        abort("InitialParValues has to be an array of three numeric values");
        return ReturnNanArray(6);
    }

    RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
    if (r.type.startsWith("TH1"))
      {
        TH1* h = static_cast<TH1*>(r.Obj);

        TF1 *f1 = new TF1("f1","[0]*exp(-0.5*((x-[1])/[2])^2)");
        f1->SetParameters(ar[0].toDouble(), ar[1].toDouble(), ar[2].toDouble());

        int status = h->Fit(f1, options.toLatin1());
        if (status != 0) return ReturnNanArray(6);

        emit RequestDraw(0, "", true); //to update

        QJsonArray ar;
        for (int i=0; i<3; i++) ar << f1->GetParameter(i);
        for (int i=0; i<3; i++) ar << f1->GetParError(i);

        QJsonValue jv = ar;
        QVariant res = jv.toVariant();
        return res;
      }
    else
      {
        abort("Object "+HistName+": unsupported histogram type!");
        return ReturnNanArray(6);
    }
}

bool AInterfaceToHist::Delete(QString HistName)
{
    return TmpHub->ScriptDrawObjects.remove(HistName);
}

void AInterfaceToHist::DeleteAllHist()
{
    TmpHub->ScriptDrawObjects.removeAllHists();
}

bool AInterfaceToHist::IsHistExist(QString HistName)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
    return (index != -1);
}

void AInterfaceToHist::SetOptStat(QString opt)
{
    gStyle->SetOptStat(opt.toLocal8Bit().data());
}

void AInterfaceToHist::Draw(QString HistName, QString options)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
  if (index == -1)
    {
      abort("Histogram "+HistName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  if (r.type == "TH1D")
    {
      TH1D* h = static_cast<TH1D*>(r.Obj);
      h->SetXTitle(r.Xtitle.toLatin1().data());
      h->SetYTitle(r.Ytitle.toLatin1().data());
//      h->SetLineColor(r.LineColor);
//      h->SetLineWidth(r.LineWidth);
//      h->SetLineStyle(r.LineStyle);
      emit RequestDraw(h, options, true);
    }
  else if (r.type == "TH2D")
    {
      TH2D* h = static_cast<TH2D*>(r.Obj);
      h->SetXTitle(r.Xtitle.toLatin1().data());
      h->SetYTitle(r.Ytitle.toLatin1().data());
      h->SetZTitle(r.Ztitle.toLatin1().data());
//      h->SetLineColor(r.LineColor);
//      h->SetLineWidth(r.LineWidth);
//      h->SetLineStyle(r.LineStyle);
      emit RequestDraw(h, options, true);
    }
  else
    {
      abort("Object "+HistName+": unknown histogram type!");
      return;
  }
}

void AInterfaceToHist::DrawStack(QVariant HistNames, QString options, QString XTitle, QString YTitle)
{
    QString typeArr = HistNames.typeName();
    if (typeArr != "QVariantList")
    {
        abort("Array with histogram names is expected as the second argument in hist.DrawStack");
        return;
    }

    QVariantList vl = HistNames.toList();
    QJsonArray arr = QJsonArray::fromVariantList(vl);
    THStack *stack = new THStack();
    for (int i=0; i<arr.size(); i++)
    {
        QString HistName = arr[i].toString();
        int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
        if (index == -1)
          {
            abort("Histogram "+HistName+" not found!");
            delete stack;
            return;
          }
        RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
        if (r.type != "TH1D")
        {
            abort("This is not a 1D histogram!");
            delete stack;
            return;
        }
        TH1D* h = static_cast<TH1D*>(r.Obj);
        //h->SetOption("bar");
        stack->Add(h);
    }



    emit RequestDraw(stack, options, true);

    if (!XTitle.isEmpty() || !YTitle.isEmpty())
    {
        stack->GetXaxis()->SetTitle(XTitle.toLocal8Bit().data());
        stack->GetYaxis()->SetTitle(YTitle.toLocal8Bit().data());
        emit RequestDraw(stack, options, true);
    }

    // ***!!! add storage / delete!!!
}

void AInterfaceToHist::Save(QString HistName, QString FileName)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(HistName);
    if (index == -1)
      {
        abort("Histogram "+HistName+" not found!");
        return;
      }

    RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
    if (r.type == "TH1D")
      {
        TH1D* h = static_cast<TH1D*>(r.Obj);
        h->SaveAs(FileName.toLocal8Bit().data());
      }
    else if (r.type == "TH2D")
      {
        TH2D* h = static_cast<TH2D*>(r.Obj);
        h->SaveAs(FileName.toLocal8Bit().data());
      }
    else
    {
        abort("Histogram not found!");
    }
}

// --------------------- End of HIST ------------------------

//----------------------------------
AInterfaceToGraph::AInterfaceToGraph(TmpObjHubClass *TmpHub)
  : TmpHub(TmpHub)
{
  H["NewGraph"] = "Creates a new graph (Root TGraph object)";
  H["SetMarkerProperties"] = "Default marker properties are 1, 20, 1";
  H["SetLineProperties"] = "Default line properties are 1, 1, 2";
  H["Draw"] = "Draws the graph (use \"APL\" options if in doubt)";
}

bool AInterfaceToGraph::InitOnRun()
{
  //TmpHub->ScriptDrawObjects.clear();
  return true;
}

void AInterfaceToGraph::NewGraph(QString GraphName)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
  if (index != -1)
    {
      abort("Bad new graph name! Object "+GraphName+" already exists");
      return;
    }

  TGraph* gr = new TGraph();
  TmpHub->ScriptDrawObjects.append(gr, GraphName, "TGraph");
  gr->SetFillColor(0);
  gr->SetFillStyle(0);
}

void AInterfaceToGraph::SetMarkerProperties(QString GraphName, int MarkerColor, int MarkerStyle, int MarkerSize)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
  if (index == -1)
    {
      abort("Graph "+GraphName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  r.MarkerColor = MarkerColor;
  r.MarkerStyle = MarkerStyle;
  r.MarkerSize = MarkerSize;
}

void AInterfaceToGraph::SetLineProperties(QString GraphName, int LineColor, int LineStyle, int LineWidth)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
  if (index == -1)
    {
      abort("Graph "+GraphName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  r.LineColor = LineColor;
  r.LineStyle = LineStyle;
  r.LineWidth = LineWidth;
}

void AInterfaceToGraph::SetTitles(QString GraphName, QString X_Title, QString Y_Title)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
  if (index == -1)
    {
      abort("Graph "+GraphName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  r.Xtitle = X_Title;
  r.Ytitle = Y_Title;
}

void AInterfaceToGraph::AddPoint(QString GraphName, double x, double y)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
  if (index == -1)
    {
      abort("Graph "+GraphName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  if (r.type == "TGraph")
    {
      TGraph* gr = static_cast<TGraph*>(r.Obj);
      gr->SetPoint(gr->GetN(), x, y);
    }
  else
    {
      abort("Graph "+GraphName+" not found!");
      return;
  }
}

void AInterfaceToGraph::AddPoints(QString GraphName, QVariant xArray, QVariant yArray)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
    if (index == -1)
      {
        abort("Graph "+GraphName+" not found!");
        return;
      }

    QString typeX = xArray.typeName();
    QString typeY = yArray.typeName();
    if (typeX != "QVariantList" || typeY != "QVariantList")
    {
        qWarning() << "arrays are expected in graph.AddPoints()";
        return;
    }

    QVariantList vx = xArray.toList();
    QVariantList vy = yArray.toList();
    QJsonArray X = QJsonArray::fromVariantList(vx);
    QJsonArray Y = QJsonArray::fromVariantList(vy);
    if (X.isEmpty() || Y.isEmpty() || X.size()!=Y.size())
    {
        qWarning() << "Empty or mismatch in add array to graph";
        return;
    }

    RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
    if (r.type == "TGraph")
      {
        TGraph* gr = static_cast<TGraph*>(r.Obj);

        for (int i=0; i<X.size(); i++)
            if (X[i].isDouble() && Y[i].isDouble())
            {
                double x = X[i].toDouble();
                double y = Y[i].toDouble();
                gr->SetPoint(gr->GetN(), x, y);
            }
      }
    else
      {
        abort("Graph "+GraphName+" not found!");
        return;
    }
}

void AInterfaceToGraph::AddPoints(QString GraphName, QVariant xyArray)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
    if (index == -1)
      {
        abort("Graph "+GraphName+" not found!");
        return;
      }

    QString typeArr = xyArray.typeName();
    if (typeArr != "QVariantList")
    {
        qWarning() << "arrays are expected in graph.AddPoints()";
        return;
    }

    QVariantList xy = xyArray.toList();
    QJsonArray XYarr = QJsonArray::fromVariantList(xy);

    RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
    if (r.type == "TGraph")
      {
        TGraph* gr = static_cast<TGraph*>(r.Obj);

        for (int i=0; i<XYarr.size(); i++)
        {
           QJsonArray el = XYarr[i].toArray();
           if (el.size() == 2)
           {
               if (el[0].isDouble() && el[1].isDouble())
               {
                   double x = el[0].toDouble();
                   double y = el[1].toDouble();
                   gr->SetPoint(gr->GetN(), x, y);
               }
           }
        }
      }
    else
      {
        abort("Graph "+GraphName+" not found!");
        return;
    }
}

void AInterfaceToGraph::Draw(QString GraphName, QString options)
{
  int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
  if (index == -1)
    {
      abort("Graph "+GraphName+" not found!");
      return;
    }

  RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
  if (r.type == "TGraph")
    {
      TGraph* gr = static_cast<TGraph*>(r.Obj);
      if (gr->GetN() == 0) return;

      gr->SetLineColor(r.LineColor);
      gr->SetLineWidth(r.LineWidth);
      gr->SetLineStyle(r.LineStyle);
      gr->SetMarkerColor(r.MarkerColor);
      gr->SetMarkerSize(r.MarkerSize);
      gr->SetMarkerStyle(r.MarkerStyle);
      gr->SetEditable(false);
      gr->GetYaxis()->SetTitleOffset((Float_t)1.30);
      emit RequestDraw(gr, options, true);

      gr->GetXaxis()->SetTitle(r.Xtitle.toLatin1().data());
      gr->GetYaxis()->SetTitle(r.Ytitle.toLatin1().data());
      emit RequestDraw(0, "", true); //to update canvas so axes titles are visible
    }
  else
    {
      abort("Graph "+GraphName+" not found!");
      return;
  }
}

void AInterfaceToGraph::Save(QString GraphName, QString FileName)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
    if (index == -1)
      {
        abort("Graph "+GraphName+" not found!");
        return;
      }

    RootDrawObj& r = TmpHub->ScriptDrawObjects.List[index];
    if (r.type == "TGraph")
    {
        TGraph* gr = static_cast<TGraph*>(r.Obj);
        gr->SaveAs(FileName.toLocal8Bit().data());
    }
}

bool AInterfaceToGraph::Delete(QString GraphName)
{
    return TmpHub->ScriptDrawObjects.remove(GraphName);
}

void AInterfaceToGraph::DeleteAllGraph()
{
    TmpHub->ScriptDrawObjects.removeAllGraphs();
}

bool AInterfaceToGraph::IsGraphExists(QString GraphName)
{
    int index = TmpHub->ScriptDrawObjects.findIndexOf(GraphName);
    return (index != -1);
}

// ------------------ END of GRAPH --------------------
