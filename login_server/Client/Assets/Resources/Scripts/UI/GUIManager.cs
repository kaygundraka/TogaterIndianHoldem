using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.UI;

public enum MESSAGE_BOX_TYPE
{
    SIMPLE,
    OK_CANCLE
}

public class GUIManager : MonoBehaviour 
{
	public static GUIManager Instance;

	void Awake()
	{
		Instance = this;
		DontDestroyOnLoad (gameObject);
	}

	void Start () 
	{

	}

	void Update ()
	{
		
	}

    public void ShowPrompt(string titleMessage, string placeHolder, UnityAction<string> answerAction)
    {
        GameObject promptWindow = (GameObject)Resources.Load ("prefabs/prompt");
        Prompt prompt = Instantiate(promptWindow).GetComponent<Prompt>();
        prompt.Initialize (titleMessage, placeHolder, answerAction);

        prompt.name = titleMessage + " Prompt";
    }

	/*public void ShowMessageBox(string message, UnityAction closeAction)
	{
		GameObject window = (GameObject)Resources.Load ("prefabs/message box");
		MessageBox messagebox = Instantiate(window).GetComponent<MessageBox> ();
		messagebox.Initialize (message, closeAction);

		messagebox.name = message + " MessageBox";
	}*/

    public void ShowMessageBox(string message, UnityAction closeAction, MESSAGE_BOX_TYPE type)
    {
        GameObject window = null;

        switch (type)
        {
            case MESSAGE_BOX_TYPE.SIMPLE:
                window = (GameObject)Resources.Load ("prefabs/message box");
                break;
            case MESSAGE_BOX_TYPE.OK_CANCLE:
                window = (GameObject)Resources.Load ("prefabs/message box(ok, cancle)");
                break;
        }

        MessageBox messagebox = Instantiate(window).GetComponent<MessageBox> ();
        messagebox.Initialize (message, closeAction);

        messagebox.name = message + " MessageBox";
    }
}
