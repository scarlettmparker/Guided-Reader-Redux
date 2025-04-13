import { Component, JSX } from "solid-js";
import styles from './header.module.css';

interface HeaderProps {
  children?: JSX.Element;
}

const Header: Component<HeaderProps> = (props) => {
  const { children } = props;

  return (
    <div class={styles.header}>
      { children ? ( children ) : '' }
    </div>
  );
}

export default Header;