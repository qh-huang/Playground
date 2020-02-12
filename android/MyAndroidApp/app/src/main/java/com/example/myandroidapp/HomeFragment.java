package com.example.myandroidapp;


import android.os.Bundle;

import androidx.fragment.app.Fragment;
import androidx.navigation.Navigation;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;


/**
 * A simple {@link Fragment} subclass.
 */
public class HomeFragment extends Fragment {

    public HomeFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, final ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_home, container, false);

        Button btnMap = view.findViewById(R.id.btn_map);
        Button btnList = view.findViewById(R.id.btn_list);
        Button btnAbout = view.findViewById(R.id.btn_about);

        btnMap.setOnClickListener(Navigation.createNavigateOnClickListener(R.id.action_homeFragment_to_mapFragment));
        btnList.setOnClickListener(Navigation.createNavigateOnClickListener(R.id.action_homeFragment_to_listFragment));
        btnAbout.setOnClickListener(Navigation.createNavigateOnClickListener(R.id.action_homeFragment_to_aboutFragment));

        return view;
    }

}
