using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.UI;

public class MessageBox : MonoBehaviour 
{
	private UnityAction closeEvent;
	private Text text;
	private Button button;
    private Button cancleButton;

	private RectTransform rectTransform;

	void Start () 
	{
		
	}

	void Update () 
	{
		
	}

    public void Initialize(string message, UnityAction action)
	{
		rectTransform = GetComponent<RectTransform> ();
        rectTransform.SetParent(GameObject.Find ("Main Panel").transform);
		rectTransform.anchoredPosition = new Vector2 (0, 0);

		button = transform.FindChild ("OK Button").GetComponent<Button> ();
		button.onClick.AddListener (CloseWindow);

        Transform cancle = transform.FindChild("Cancle Button");

        if (cancle != null)
        {
            cancleButton = cancle.GetComponent<Button>();
            cancleButton.onClick.AddListener(Cancle);
        }

        closeEvent = action;

		text = transform.FindChild ("Text").GetComponent<Text> ();
		text.text = message;
	}

	private void CloseWindow()
	{
		if(closeEvent != null)
			closeEvent ();
		
		Destroy (gameObject);
	}

    private void Cancle()
    {
        Destroy (gameObject);
    }
}
