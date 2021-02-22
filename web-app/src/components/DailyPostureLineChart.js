import React from 'react';
import {Chart, Line} from 'react-chartjs-2';
import 'chartjs-plugin-annotation';
import 'chartjs-plugin-deferred';

Chart.defaults.global.defaultFontColor = '#fafafa';
Chart.defaults.global.defaultFontFamily = "'Monserrat', sans-serif";

const DailyPostureLineChart = ({
    distanceVals,
    awayDesk
}) => {

    var UpperLine = 1000//100cm-1000mm
    var LowerLine = 500//50cm-500mm
    var MidPoint = 750
    var labels = []
    var MaxVal = distanceVals[0]
    for (let i = 0; i < distanceVals.length; i++) {
        labels[i] = i*10
        //don't add point if away from desk
        if (awayDesk[i] == false && distanceVals[i] < 2000){
            if(distanceVals[i] > MaxVal){
                MaxVal = distanceVals[i] 
            }         
        }
        else{
            distanceVals[i] = null
        }
    }
   

    return (
    <div className="container">
        <Line
        data={{
            labels: labels,
            datasets: [
                {
                label: 'Posture',
                backgroundColor: 'rgba(0, 0, 0, 0)',
                borderColor: '#fafafa',
                borderDash:[2],
                pointBackgroundColor: function(context) {
                    var index = context.dataIndex;
                    var value = context.dataset.data[index];
                    return value > UpperLine ? '#e6214f' : 
                        value < LowerLine ? '#e6214f' : 
                        '#51cda0';
                },
                pointBorderColor: 'rgba(0, 0, 0, 0)',
                pointRadius: 5,
                borderWidth: 2,
                data: distanceVals
                }
            ]
        }}
        options={{
            responsive:true,
            spanGaps: true,
            legend:{
                display:false,
            },
            scales:{
                yAxes:[{
                    gridLines:{
                        display:false
                    },
                    ticks:{
                        display:false,
                        stepSize:1,
                        beginAtZero:true,
                        max: Math.max(MaxVal+100, 1150)
                }
                }],
                xAxes:[{
                    gridLines:{
                        zeroLineWidth:3
                    },
                    ticks:{
                        display:false,
                        fontSize:14
                    }
                }]
            },
            annotation:{
                annotations:[
                    {
                    borderColor:'black',
                    borderWidth:2,
                    borderDash:[10],
                    mode:'horizontal',
                    type:'line',
                    value:MidPoint,
                    scaleID:'y-axis-0'
                    }, {
                    borderColor:'#6d78ad',
                    borderWidth:2,
                    borderDash:[10],
                    mode:'horizontal',
                    type:'line',
                    value:UpperLine,
                    scaleID:'y-axis-0',
                    label: {
                        backgroundColor: "rgba(0, 0, 0, 0)",
                        position: 'left',
                        content: "Too Far",
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
                        position: 'left',
                        content: "Too Close",
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

export default DailyPostureLineChart