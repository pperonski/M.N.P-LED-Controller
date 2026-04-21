    let networks=JSON.parse("{}");

    let index=0;
    const NETWORK_COUNTS=10;

    var scanSSIDRoutine=null;
    var readSSIDRoutine=null;

    checkParameters();

    // function to draw wifi signal strength icon , onto canvas canv based on rssi
    function drawWiFiStrength(canv,rssi)
    {
        if((Number.isInteger(rssi))&&(rssi<0))
        {
            rssi=100+rssi;
        }

        const width=canv.width;
        const height=canv.height;
        let ctx=canv.getContext("2d");

        ctx.arc(width/2,0.4*height,0.1*width,0,2*Math.M_PI);

        ctx.fill();

        if(rssi>20)
        {
            ctx.beginPath();
            ctx.moveTo(0.3*width,0.7*height);
            ctx.quadraticCurveTo(width/2,0.6*height,0.3*width,0.7*height);
            ctx.stroke();
        }
    }

    // convert rssi to procent , later it will be replaced with proper icon
    function from_rssi_to_percent(rssi)
    {
        if(Number.isInteger(rssi))
        {
            return 100+rssi;
        }
    }

    // display networks in list's div
    function display_networks(wlans,output)
    {
        wlans.forEach(obj => 
            {
                let head;
                if(!Boolean(obj.connected))
                    {
                    head="<div class='list_entry wlan_entry'>";
                    }
                    else
                    {
                    head="<div class='list_entry wlan_connected'>";
                    }


                let butt1;

                if(!Boolean(obj.connected))
                {
                    butt1="<button onclick=connect(\""+obj.ssid+"\")>Connect</button>";
                }
                else
                {
                    butt1="<button onclick=disconnect()>Disconnect</button>";
                }

                console.log(obj);
                output.innerHTML+= head
                + "<p>"
                + from_rssi_to_percent(obj.rssd) + "% "
                + "</p>"
                + "<p>"
                + obj.ssid 
                + "</p>"
                + butt1
                + "</div>";
            })
          
    }


// function to read scanned SSIDS from ESP
    async function get_SSID()
    {

        const wlan_list=document.getElementById("wifi");

        await fetch(`/1234?index=${index}&counts=${NETWORK_COUNTS}`,
        {
            method:"GET"
        }).then(res => res.json())
        .then(res => {
            networks=res;

            console.log(networks);
            wlan_list.innerHTML="";
            display_networks(networks,wlan_list);
           
                //wlan_list.innerHTML="<div class='list_entry'>No networks!</div>";
            
        })
        .catch(error => {
            //wlan_list.innerHTML="<div class='list_entry'>Connection error!</div>";
            show_msg(error);
            wlan_list.innerHTML="";
            console.error(error);
        });

    }

// function to start network scan
    async function scanSSID()
    {

        await fetch("/1235",
        {
            method:"GET"
        })
        .then(res =>{
            if(res.ok)
            {
                console.log("Network scan started!");
            }
        })
        .catch(error => {
            show_msg(error);
            wlan_list.innerHTML="";
            console.error(error);
        });
        
    }

    async function disconnect()
    {
        await fetch("/3570",
        {
            method:"DELETE"
        }).then(res => res.text())
        .then(res => {
            console.log(res);
            show_msg(res);
             
            reload();
        })
        .catch(error => {
            //wlan_list.innerHTML="<div class='list_entry'>Connection error!</div>";
            show_msg(error);
            console.error(error);
        });
    }

    // send connect command to esp with ssid and password from wlan_form
    function connect(ssid)
    {
        if(!(typeof ssid === 'string'))
        {
            show_msg("SSID must be string!");
            return;
        }
        document.forms["wlan_form"]["ssid"].value = ssid;
        document.forms["wlan_form"]["pwd"].value='';
        document.getElementById("password").style.display="block";
        

    }

    // send password from wlan
    async function submit_pwd()
    {
        const ssid=document.forms["wlan_form"]["ssid"].value;
        const pwd=document.forms["wlan_form"]["pwd"].value;

        closeForm();

        if(!(document.getElementById("wifi")===null))
        {

        await fetch("/2441",
        {
            method: "POST",
            headers: {"ssid":ssid,"password":pwd}

        })
        .then(res => 
            {
                if(res.ok)
                {
                    show_msg("Network has been registered!");

                    get_SSID();
                }
            })
        .catch(error=>{show_msg(error.message)});

        }

        if(!(document.getElementById("net")===null))
        {

        await fetch("/2445",
        {
            method: "POST",
            headers: {"ssid":ssid,"password":pwd}

        })
        .then(res => 
            {
                if(res.ok)
                {
                    show_msg("Network has been updated!");
                    load_list();
                }
            })
        .catch(error=>{show_msg(error.message)});

        }
        
    }

    function show_msg(msg)
    {
       const box=document.getElementById("msg");
       box.innerHTML=msg;
       box.style.display="block";

    }

    function hide_msg(msg)
    {
        msg.style.display="none";
    }

    function closeForm()
    {
        document.getElementById("password").style.display="none";
    }


    function reload()
    {
        StopScanRoutine();

        get_SSID();

        StartScanRoutine();
    }

    function page_up()
    {
        index+=NETWORK_COUNTS;
    }

    function page_down()
    {
        index-=NETWORK_COUNTS;

        if(index<0)
        {
            index=0;
        }
    }

    function StartScanRoutine()
    {
        readSSIDRoutine=setInterval(get_SSID,60*1000);
        scanSSIDRoutine=setInterval(scanSSID,30*1000);
    }

    function StopScanRoutine()
    {
        clearInterval(readSSIDRoutine);
        clearInterval(scanSSIDRoutine);
    }

    // load connect sub menu
    function load_connect()
    {
        if(document.getElementById("wifi")=== null)
        {
        
        index=0;

        const overview=document.getElementById("overview");

        overview.innerHTML="<div id='wifi' class='list'></div>";
        overview.innerHTML+="<button class='refresh' onclick='reload()'>Refresh</button>";
        overview.innerHTML+="<button class='refresh' onclick='page_up()'>NEXT</button>";
        overview.innerHTML+="<button class='refresh' onclick='page_down()'>BACK</button>";

        StartScanRoutine();

        scanSSID();
        }
    }


    // update ssid
    async function update(ssid,password)
    {
       await fetch("/2445",
        {
            method: "POST",
            headers: {"ssid":ssid,"password":password}

        })
        .then(res => {
            if(res.ok)
            {
                show_msg("Network has been updated!");
            }
        })
        .catch(error=>{show_msg(error.message)});
    }

    // remove SSID
    function remove_SSID(ssid)
    {
        fetch("/3561",
        {
            method: "DELETE",
            headers: {"ssid":ssid}

        })
        .then(res => {
            if(res.ok)
            {
                show_msg("Network has been removed!");
            }
        })
        .catch(error=>{show_msg(error.message)});
    }

    // display networks in list's div
    function display_save_networks(wlans,output)
    {
        wlans.forEach(obj => 
            {
                let head;
                if(!Boolean(obj.connected))
                    {
                    head="<div class='list_entry wlan_entry'>";
                    }
                    else
                    {
                    head="<div class='list_entry wlan_connected'>";
                    }


                let butt1;

                
                butt1="<button onclick=remove_SSID('"+obj.ssid+"')>Remove</button>";

                butt1+="<button onclick=connect('"+obj.ssid+"')>Update</button>";
                

                console.log(obj);
                output.innerHTML+= head
                + "<p>"
                + obj.ssid 
                + "</p>"
                + butt1
                + "</div>";
            })
          
    }

    async function load_list()
    {
        const wlan_list=document.getElementById("net");

        
        await fetch(`/1456?index=${index}&counts=${NETWORK_COUNTS}`,
        {
            method:"GET"
        }).then(res => res.json())
        .then(res => {
            networks=res;

            console.log(networks);
            wlan_list.innerHTML="";
            display_save_networks(networks,wlan_list);
           
                //wlan_list.innerHTML="<div class='list_entry'>No networks!</div>";
            
        })
        .catch(error => {
            //wlan_list.innerHTML="<div class='list_entry'>Connection error!</div>";
            show_msg(error.message);
            wlan_list.innerHTML="";
            console.error(error);
        });
    }

    function list_networks()
    {
        if(document.getElementById("net")=== null)
        {
        index=0;
        StopScanRoutine();
        const overview=document.getElementById("overview");

        overview.innerHTML="<div id='net' class='list'></div>";
        overview.innerHTML+="<button class='refresh' onclick='load_list()'>Refresh</button>";
        overview.innerHTML+="<button class='refresh' onclick='page_up()'>NEXT</button>";
        overview.innerHTML+="<button class='refresh' onclick='page_down()'>BACK</button>";

        load_list();

        }
    }

    async function checkParameters()
    {
        const PARAMETERS_NUMBER = await getParameterNumber();

        console.log("Parameter number: ",PARAMETERS_NUMBER);

        if(PARAMETERS_NUMBER<=0)
        {
            document.getElementById("parameters").style.display="none";
        }
    }

    // read and return number of parameters
    async function getParameterNumber()
    {
        

        return await fetch("/6236",
        {
            method:"GET"
        }).then(res => res.text())
        .then(res => {
            
           if(!isNaN(res))
           {
                return Number.parseInt(res);
           }

           return 0;
                //wlan_list.innerHTML="<div class='list_entry'>No networks!</div>";
           
        })
        .catch(error => {
            console.error(error);
            return -1;
        });


    }


    async function load_params() 
    {
        if(document.getElementById("params") === null)
        {

        StopScanRoutine();
        const overview=document.getElementById("overview");
        overview.innerHTML="<form id='params' class='list'></form> <br>";
        overview.innerHTML+="<button onclick='send_params()'>Send</button>";

        const param_list=document.getElementById("params");
        param_list.innerHTML="";

        const PARAMETERS_NUMBER = await getParameterNumber();

        if(PARAMETERS_NUMBER==-1)
        {
            load_connect();
            return;
        }

        console.log("Parameter number: ",PARAMETERS_NUMBER);


        for(let i=0; i<PARAMETERS_NUMBER; i++)
        {

        await fetch("/6244?id="+i,
        {
            method:"GET"
        }).then(res => res.json())
        .then(res => {

            console.log(res);

            param_list.innerHTML+=`<p>${res.description}</p>`;
            param_list.innerHTML+=`<input type='${res.type}' value='${res.default}'
            maxlength='${res.maxLength}' minlength='${res.minLength}' '${res.additional_arguments}'>`;
            
            
        })
        .catch(error => {
            show_msg(error);
            console.error(error);
        
        });

    }
        }
    }

    async function send_params()
    {
        const param_list=document.getElementById("params");

        if(param_list === null)
        {
            return;
        }

        const inputlist=param_list.childNodes;

        for(let i=0; i<inputlist.length; i++)
        {
            if(inputlist[i].nodeName!="input")
            {
                continue;
            }
            console.log(inputlist[i]);
            await fetch("/6213?id="+i,
                    {
                        method:"POST",
                        headers:{"value":inputlist[i].value}
                    })
                    .then(res => res.text())
                    .then(res=>
                        {
                            show_msg(res);
                        })
                    .catch(err => {
                        show_msg(err);
                    });
        }

        await fetch("/6256",
                    {
                        method:"POST"
                    })
                    .then(res => 
                        {
                            if(res.ok)
                            {
                                show_msg("Parameters have been updated!");
                            }
                        })
                    .catch(err => {
                        show_msg(err);
                    });



    }


    load_connect();