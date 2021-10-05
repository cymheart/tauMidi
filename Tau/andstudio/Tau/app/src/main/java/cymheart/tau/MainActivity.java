package cymheart.tau;

import androidx.appcompat.app.AppCompatActivity;
import android.Manifest;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import com.tbruyelle.rxpermissions3.RxPermissions;

import cymheart.tau.effect.Equalizer;

public class MainActivity extends AppCompatActivity {

    Tau tau;
    VirInstrument virInst;
    Equalizer eq;
    MidiEditor midiPlay;

    private static final String[] permissionsGroup=
            new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE};


    class ButtonListener implements View.OnClickListener, View.OnTouchListener {

        public void onClick(View v) {
            if(v.getId() == R.id.button2){
                Log.d("test", "cansal button ---> click");
            }
        }

        public boolean onTouch(View v, MotionEvent event) {
            if(v.getId() == R.id.button2){
                if(event.getAction() == MotionEvent.ACTION_UP){
                    tau.OffKey(60, 127, virInst);
                }
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    tau.OnKey(60, 127, virInst);
                }
            }

            if(v.getId() == R.id.button3){
                if(event.getAction() == MotionEvent.ACTION_UP){
                    tau.OffKey(40, 127, virInst);
                }
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    tau.OnKey(40, 127, virInst);
                }
            }

            if(v.getId() == R.id.button4){
                if(event.getAction() == MotionEvent.ACTION_UP){
                    tau.OffKey(90, 127, virInst);
                }
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    tau.OnKey(90, 127, virInst);
                }
            }

            if(v.getId() == R.id.button5){
                if(event.getAction() == MotionEvent.ACTION_UP){
                    tau.OffKey(110, 127, virInst);
                }
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    tau.OnKey(110, 127, virInst);
                }
            }
            return false;
        }

    }

    void cb(VirInstrument[] virInst)
    {
        //cmd.OnKey(60, 127, virInst[0]);

        int s = virInst.length;

    }

    AppCompatActivity activityw;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        Button btn = findViewById(R.id.button);


            System.loadLibrary("tau");

        SoundFont sf = new SoundFont();
        sf.Parse("SF2", "/storage/emulated/0/gnusmas_gm_soundfont_2.01.sf2");
        tau = new Tau(this);


      //  eq = new Equalizer();
       // Filter[] filters = eq.GetFilter();

//        Bode bode = new Bode();
//        bode.AddFilters(filters);
//
//        bode.SetSampleFreq(44100);
//        bode.SetFreqzSampleCount(500);
//
//        bode.SetPlotAreaHeight(1080);
//        bode.SetPlotAreaWidth(354);
//
//        bode.SetPlotFreqAxisStart(0);
//        bode.SetPlotFreqAxisEnd(44100/2);
//
//        bode.SetPlotGainDbRange(30);
//
//        bode.Compute();

       // float[] xpos = bode.GetPlotFreqAxisPos();
       // float[] ypos = bode.GetPlotGainAxisPos();


        //eqCmd = new EffectEqualizerCmd(ventrue, eq);
        // ventrueCmd.AddEffect(eq);

       // eqCmd.SetFreqBandGain(0, 1);

        tau.SetFrameSampleCount(4096);
        tau.SetChannelCount(2);
        tau.SetSoundFont(sf);
        tau.Open();

        ButtonListener b = new ButtonListener();

        Button btn2 = findViewById(R.id.button2);
        btn2.setOnTouchListener(b);

        Button btn3 = findViewById(R.id.button3);
        btn3.setOnTouchListener(b);

        Button btn4 = findViewById(R.id.button4);
        btn4.setOnTouchListener(b);

        Button btn5 = findViewById(R.id.button5);
        btn5.setOnTouchListener(b);


        // tv.setText(stringFromJNI());
        activityw = this;
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view)
            {
                RxPermissions permissions = new RxPermissions(activityw);
                permissions.setLogging(true);
                permissions.request(permissionsGroup)
                        .subscribe(
                                granted -> {
                                    if (granted) { // Always true pre-M
                                        // I can control the camera now
                                        //tau.ParseSoundFont("SF2", "/storage/emulated/0/gnusmas_gm_soundfont_2.01.sf2");

                                         //cmd.AppendMidiFile("/storage/emulated/0/(ACG)芬兰梦境乐团-The Dawn《魔兽世界》亡灵序曲.mid");

                                        //VirInstrument inst = tau.EnableVirInstrument(0, 0, 0, 25);
                                        //tau->SetVirInstrumentMidiControllerValue(inst, MidiControllerType::ModulationWheelMSB, 127);
                                       // tau.OnKey(60, 127, inst);


                                        tau.AppendMidiFile("/storage/emulated/0/紅蓮の弓矢.mid");
                                        midiPlay = tau.LoadMidi("/storage/emulated/0/紅蓮の弓矢.mid", true);
                                        tau.PlayMidi();
//                                        int s = tau.GetMidiState();
//                                        Log.d("state", s+"");
//                                        tau.PauseMidi();
//                                        s = tau.GetMidiState();
//                                        Log.d("state", s+"");
//
//                                        Thread.sleep(2000);
//
//                                        tau.PlayMidi();
//                                        s = tau.GetMidiState();
//                                        Log.d("state", "statew:" +s+"");
                                       // virInst = cmd.EnableVirInstrument(0,0,0,0);
                                        //cmd.OnKey(60, 127, virInst);

                                        //Ventrue.ndkTest();

                                    } else {
                                        //TestVentrue.VentruePlayTest();

                                    }
                                });
            }
        });
        // checkPermissionRequest(this);
        //testRequest(tv);
    }





//    /**
//     * request����:
//     * ��֧�ַ���Ȩ����;
//     * ���ص�Ȩ�޽��:ȫ��ͬ��ʱֵtrue,����ֵΪfalse
//     */
//    public void testRequest(View view) {
//        RxPermissions rxPermissions = new RxPermissions(this);
//        rxPermissions.request(permissionsGroup)
//                .subscribe(granted -> {
//                    Log.i("cxw","������:"+granted);
//                });
//
//    }
//
//    public void checkPermissionRequest(FragmentActivity activity) {
//        RxPermissions permissions = new RxPermissions(activity);
//        permissions.setLogging(true);
//        permissions.request(Manifest.permission.READ_EXTERNAL_STORAGE)
//                .subscribe(
//                        granted -> {
//                    if (granted) { // Always true pre-M
//                        // I can control the camera now
//                     //   TestVentrue.VentruePlayTest();
//
//                    } else {
//                      //  TestVentrue.VentruePlayTest();
//
//                    }
//                });
//    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    //public native String stringFromJNI();





}
