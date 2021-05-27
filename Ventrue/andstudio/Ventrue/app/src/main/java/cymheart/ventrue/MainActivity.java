package cymheart.ventrue;

import androidx.appcompat.app.AppCompatActivity;
import android.Manifest;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import com.tbruyelle.rxpermissions3.RxPermissions;

import cymheart.ventrue.R;
import cymheart.ventrue.Ventrue;
import cymheart.ventrue.VentrueCmd;
import cymheart.ventrue.VirInstrument;
import cymheart.ventrue.dsignal.Bode;
import cymheart.ventrue.dsignal.Filter;
import cymheart.ventrue.effect.EffectEqualizerCmd;
import cymheart.ventrue.effect.Equalizer;

public class MainActivity extends AppCompatActivity {

    Ventrue ventrue;
    VentrueCmd cmd;
    VirInstrument virInst;
    Equalizer  eq;
    EffectEqualizerCmd  eqCmd;
    MidiPlay midiPlay;

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
                    cmd.OffKey(60, 127, virInst);
                }
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    cmd.OnKey(60, 127, virInst);
                }
            }

            if(v.getId() == R.id.button3){
                if(event.getAction() == MotionEvent.ACTION_UP){
                    cmd.OffKey(40, 127, virInst);
                }
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    cmd.OnKey(40, 127, virInst);
                }
            }

            if(v.getId() == R.id.button4){
                if(event.getAction() == MotionEvent.ACTION_UP){
                    cmd.OffKey(90, 127, virInst);
                }
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    cmd.OnKey(90, 127, virInst);
                }
            }

            if(v.getId() == R.id.button5){
                if(event.getAction() == MotionEvent.ACTION_UP){
                    cmd.OffKey(110, 127, virInst);
                }
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    cmd.OnKey(110, 127, virInst);
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

        ventrue = new Ventrue();
        ventrue.SetSoundEndVirInstCallBack(this::cb);


        cmd = ventrue.GetCmd();

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

         ventrue.SetFrameSampleCount(4096);
        ventrue.SetChannelCount(2);
        ventrue.OpenAudio();

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
                                        ventrue.ParseSoundFont("SF2", "/storage/emulated/0/GeneralUser GS MuseScore v1.442.sf2");

                                         //cmd.AppendMidiFile("/storage/emulated/0/(ACG)芬兰梦境乐团-The Dawn《魔兽世界》亡灵序曲.mid");

                                        cmd.AppendMidiFile("/storage/emulated/0/英雄的黎明钢琴版.mid");
                                        midiPlay = cmd.LoadMidi(0, true);
                                         //cmd.PlayMidi(0);

                                        virInst = cmd.EnableVirInstrument(0,0,0,0);
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
