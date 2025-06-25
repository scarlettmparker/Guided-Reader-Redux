import LoginForm from "~/components/routes/login/login-form";
import styles from "./login.module.css";

const Login: React.FC = () => {
  return (
    <>
      <LoginForm />
      <div className={styles.login_wrapper} />
    </>
  );
};

export default Login;
