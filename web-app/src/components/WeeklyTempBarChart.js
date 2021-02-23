import React from 'react';
import {Chart, Bar} from 'react-chartjs-2';
import firebase from '.././firebase.js';
import 'chartjs-plugin-annotation';
import 'chartjs-plugin-deferred';

Chart.defaults.global.defaultFontColor = '#fafafa';
Chart.defaults.global.defaultfontFamily = "'Monserrat', sans-serif";

const WeeklyTempBarChart = ({
  tempData,
  timeData,
  awayDesk
}) => {


  //set empty data array
  var BarData = []
  const UpperLine = 23
  const LowerLine = 19
  //Average Temperature each day
  var MonTempSum=0, TueTempSum=0, WedTempSum=0, ThuTempSum=0, FriTempSum=0, SatTempSum=0, SunTempSum=0
  var MonDataLength=0, TueDataLength=0, WedDataLength=0, ThuDataLength=0, FriDataLength=0, SatDataLength=0, SunDataLength=0
  var MaxVal = tempData[0]
  for (let i = 0; i < timeData.length; i++) {
    let DataDate = new Date(timeData[i] * 1000)
    //don't add point if away from desk
    if (awayDesk[i] == false){
      //Monday
      if(DataDate.getDay() == 1){
        MonTempSum += tempData[i]
        MonDataLength++
      }
      //Tuesday
      if(DataDate.getDay() == 2){
        TueTempSum += tempData[i]
        TueDataLength++
      }
      //Wednesday
      if(DataDate.getDay() == 3){
        WedTempSum += tempData[i]
        WedDataLength++
      }
      //Thursday
      if(DataDate.getDay() == 4){
        ThuTempSum += tempData[i]
        ThuDataLength++
      }
      //Friday
      if(DataDate.getDay() == 5){
        FriTempSum += tempData[i]
        FriDataLength++
      }
      //Saturday
      if(DataDate.getDay() == 6){
        SatTempSum += tempData[i]
        SatDataLength++
      }
      //Sunday
      if(DataDate.getDay() == 0){
        SunTempSum += tempData[i]
        SunDataLength++
      }
      //To push up graph
      if(tempData[i] > MaxVal){
        MaxVal = tempData[i] 
      }
    }
  }

  BarData[0] = (MonTempSum / MonDataLength).toFixed(1)
  BarData[1] = (TueTempSum / TueDataLength).toFixed(1)
  BarData[2] = (WedTempSum / WedDataLength).toFixed(1)
  BarData[3] = (ThuTempSum / ThuDataLength).toFixed(1)
  BarData[4] = (FriTempSum / FriDataLength).toFixed(1)
  BarData[5] = (SatTempSum / SatDataLength).toFixed(1)
  BarData[6] = (SunTempSum / SunDataLength).toFixed(1)

  //set bar colours
  var colours = []
  for (let i = 0; i < BarData.length; i++) {
    var colour = '#51cda0'
    if(BarData[i] > UpperLine){
      colour = '#e6214f'
    }
    if(BarData[i] < LowerLine){
      colour = '#6d78ad'
    }
    colours[i] = colour
  }

  return (
    <div>
      <Bar
        data={{
          labels: ['M', 'T', 'W', 'T',
                  'F', 'S', 'S'],  
          datasets: [
            {
            label: 'Average Temp',
            backgroundColor: colours,
            borderColor: 'rgba(0, 0, 0, 0)',
            borderWidth: 1,
            data: BarData
            }
          ]
        }}
        options={{
          responsive:true,
          legend:{
            display:false,
          },
          scales:{
            yAxes:[{
              gridLines:{
                zeroLineWidth:3,
                zeroLineColor:'#98999e'
              },
              ticks:{
                fontSize:14,
                fontColor:'#98999e',
                stepSize:1,
                beginAtZero:false,
                suggestedMin: 10,
                max: Math.max(MaxVal+1, 26)
              }
            }],
            xAxes:[{
              gridLines:{
                zeroLineWidth:3,
                zeroLineColor:'#98999e'
              },
              ticks:{
                fontSize:14,
                fontColor:'#98999e'
              }
            }]
          },
          annotation:{
            annotations:[{
              borderColor:'#6d78ad',
              borderWidth:2,
              borderDash:[10],
              mode:'horizontal',
              type:'line',
              value:UpperLine,
              scaleID:'y-axis-0',
              label: {
                  backgroundColor: "rgba(0, 0, 0, 0)",
                  fontColor: '#98999e',
                  position: 'left',
                  content: "Too Hot",
                  enabled: true,
                  yAdjust: -10

              }}, {
              borderColor:'#6d78ad',
              borderWidth:2,
              borderDash:[10],
              mode:'horizontal',
              type:'line',
              value:LowerLine,
              scaleID:'y-axis-0',
              label: {
                  backgroundColor: "rgba(0, 0, 0, 0)",
                  fontColor: '#98999e',
                  position: 'left',
                  content: "Too Cold",
                  enabled: true,
                  yAdjust: 12
              }}
            ]
          },
          plugins:{
            deferred: {
              yOffset:'30%',
              delay:250     
            }
          }
        }}
      />
    </div>
  );
}


export default WeeklyTempBarChart