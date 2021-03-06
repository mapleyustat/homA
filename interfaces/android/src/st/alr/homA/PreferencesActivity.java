
package st.alr.homA;

import st.alr.homA.support.Events;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.view.Menu;
import de.greenrobot.event.EventBus;

public class PreferencesActivity extends PreferenceActivity {
    private static Preference serverPreference;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Start service if it is not already started
        Intent service = new Intent(this, MqttService.class);
        startService(service);

        // Register for connection changed events
        EventBus.getDefault().register(this);

        // Replace content with fragment for custom preferences
        getFragmentManager().beginTransaction().replace(android.R.id.content, new CustomPreferencesFragment()).commit();

    }

    public static class CustomPreferencesFragment extends PreferenceFragment {

        @Override
        public void onCreate(Bundle savedInstanceState) {

            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.preferences);

            PackageManager pm = this.getActivity().getPackageManager();
            Preference version = findPreference("versionReadOnly");

            try {
                version.setSummary(pm.getPackageInfo(this.getActivity().getPackageName(), 0).versionName);
            } catch (NameNotFoundException e) {
                e.printStackTrace();
            }

            serverPreference = findPreference("serverPreference");
            setServerPreferenceSummary();

        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    public void onEventMainThread(Events.MqttConnectivityChanged event) {
        setServerPreferenceSummary();
    }

    private static void setServerPreferenceSummary() {
        serverPreference.setSummary(MqttService.getConnectivityText());
    }


    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        return true;
    }

    @Override
    public boolean onIsMultiPane() {
        return false;
    }

}
